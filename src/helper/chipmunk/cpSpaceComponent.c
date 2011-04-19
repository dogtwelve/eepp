/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chipmunk_private.h"

#pragma mark Sleeping Functions

void
cpSpaceActivateBody(cpSpace *space, cpBody *body)
{
	if(space->locked){
		// cpSpaceActivateBody() is called again once the space is unlocked
		cpArrayPush(space->rousedBodies, body);
	} else {
		cpArrayPush(space->bodies, body);

		CP_BODY_FOREACH_SHAPE(body, shape){
			cpSpatialIndexRemove(space->staticShapes, shape, shape->hashid);
			cpSpatialIndexInsert(space->activeShapes, shape, shape->hashid);
		}
		
		CP_BODY_FOREACH_ARBITER(body, arb){
			cpBody *bodyA = arb->body_a;
			if(body == bodyA || cpBodyIsStatic(bodyA)){
				int numContacts = arb->numContacts;
				cpContact *contacts = arb->contacts;
				
				// Restore contact values back to the space's contact buffer memory
				arb->contacts = cpContactBufferGetArray(space);
				memcpy(arb->contacts, contacts, numContacts*sizeof(cpContact));
				cpSpacePushContacts(space, numContacts);
				
				cpShape *a = arb->a, *b = arb->b;
				cpShape *shape_pair[] = {a, b};
				cpHashValue arbHashID = CP_HASH_PAIR((size_t)a, (size_t)b);
				cpHashSetInsert(space->cachedArbiters, arbHashID, shape_pair, arb, NULL);
				cpArrayPush(space->arbiters, arb);
				arb->stamp = space->stamp;
				
				cpfree(contacts);
			}
		}
		
		CP_BODY_FOREACH_CONSTRAINT(body, constraint){
			cpBody *bodyA = constraint->a;
			if(body == bodyA || cpBodyIsStatic(bodyA)) cpArrayPush(space->constraints, constraint);
		}
	}
}

static void
cpSpaceDeactivateBody(cpSpace *space, cpBody *body)
{
	cpArrayDeleteObj(space->bodies, body);
	
	CP_BODY_FOREACH_SHAPE(body, shape){
		cpSpatialIndexRemove(space->activeShapes, shape, shape->hashid);
		cpSpatialIndexInsert(space->staticShapes, shape, shape->hashid);
	}
	
	CP_BODY_FOREACH_ARBITER(body, arb){
		cpBody *bodyA = arb->body_a;
		if(body == bodyA || cpBodyIsStatic(bodyA)){
			cpShape *a = arb->a, *b = arb->b;
			cpShape *shape_pair[] = {a, b};
			cpHashValue arbHashID = CP_HASH_PAIR((size_t)a, (size_t)b);
			cpHashSetRemove(space->cachedArbiters, arbHashID, shape_pair);
			cpArrayDeleteObj(space->arbiters, arb);
			
			// Save contact values to a new block of memory so they won't time out
			size_t bytes = arb->numContacts*sizeof(cpContact);
			cpContact *contacts = (cpContact *)cpmalloc(bytes);
			memcpy(contacts, arb->contacts, bytes);
			arb->contacts = contacts;
		}
	}
		
	CP_BODY_FOREACH_CONSTRAINT(body, constraint){
		cpBody *bodyA = constraint->a;
		if(body == bodyA || cpBodyIsStatic(bodyA)) cpArrayDeleteObj(space->constraints, constraint);
	}
}

static inline cpBody *
ComponentRoot(cpBody *body)
{
	return (body ? body->node.root : NULL);
}

static inline void
ComponentActivate(cpBody *root)
{
	if(!root || !cpBodyIsSleeping(root)) return;
	cpAssert(!cpBodyIsRogue(root), "Internal Error: ComponentActivate() called on a rogue body.");
	
	cpSpace *space = root->space;
	CP_BODY_FOREACH_COMPONENT(root, body){
		body->node.root = NULL;
		cpSpaceActivateBody(space, body);
	}
	
	cpArrayDeleteObj(space->sleepingComponents, root);
}

static inline void
ComponentAdd(cpBody *root, cpBody *body){
	body->node.root = root;

	if(body != root){
		body->node.next = root->node.next;
		root->node.next = body;
	}
}

void
cpBodyActivate(cpBody *body)
{
	if(!cpBodyIsRogue(body)){
		body->node.idleTime = 0.0f;
		ComponentActivate(ComponentRoot(body));
	}
}

static inline cpBool
ComponentActive(cpBody *root, cpFloat threshold)
{
	CP_BODY_FOREACH_COMPONENT(root, body){
		if(body->node.idleTime < threshold) return cpTrue;
	}
	
	return cpFalse;
}

static inline void
FloodFillComponent(cpBody *root, cpBody *body)
{
	if(!cpBodyIsStatic(body) && !cpBodyIsRogue(body)){
		cpBody *other_root = ComponentRoot(body);
		if(other_root == NULL){
			ComponentAdd(root, body);
			CP_BODY_FOREACH_ARBITER(body, arb) FloodFillComponent(root, (body == arb->body_a ? arb->body_b : arb->body_a));
			CP_BODY_FOREACH_CONSTRAINT(body, constraint) FloodFillComponent(root, (body == constraint->a ? constraint->b : constraint->a));
		} else {
			cpAssert(other_root == root, "Internal Error: Inconsistency dectected in the contact graph.");
		}
	}
}

static inline void
cpBodyPushArbiter(cpBody *body, cpArbiter *arb)
{
	if(!cpBodyIsStatic(body) && !cpBodyIsRogue(body)){
		if(body == arb->body_a){
			arb->next_a = body->arbiterList;
		} else {
			arb->next_b = body->arbiterList;
		}
		
		body->arbiterList = arb;
	}
}

void
cpSpaceProcessComponents(cpSpace *space, cpFloat dt)
{
	cpFloat dv = space->idleSpeedThreshold;
	cpFloat dvsq = (dv ? dv*dv : cpvlengthsq(space->gravity)*dt*dt);
	
	// update idling and reset arbiter list and component nodes
	cpArray *bodies = space->bodies;
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody*)bodies->arr[i];
		
		// Need to deal with infinite mass objects
		cpFloat keThreshold = (dvsq ? body->m*dvsq : 0.0f);
		body->node.idleTime = (cpBodyKineticEnergy(body) > keThreshold ? 0.0f : body->node.idleTime + dt);
		
		body->arbiterList = NULL;
		body->node.next = NULL;
	}
	
	// Awaken any sleeping bodies found and then push arbiters to the bodies' lists.
	cpArray *arbiters = space->arbiters;
	for(int i=0, count=arbiters->num; i<count; i++){
		cpArbiter *arb = (cpArbiter*)arbiters->arr[i];
		cpBody *a = arb->body_a, *b = arb->body_b;
		
		if((cpBodyIsRogue(b) && !cpBodyIsStatic(b)) || cpBodyIsSleeping(a)) cpBodyActivate(a);
		if((cpBodyIsRogue(a) && !cpBodyIsStatic(a)) || cpBodyIsSleeping(b)) cpBodyActivate(b);
		
		cpBodyPushArbiter(a, arb);
		cpBodyPushArbiter(b, arb);
	}
	
	// Bodies should be held active if connected by a joint to a non-static rouge body.
	cpArray *constraints = space->constraints;
	for(int i=0; i<constraints->num; i++){
		cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
		cpBody *a = constraint->a, *b = constraint->b;
		
		if(cpBodyIsRogue(b) && !cpBodyIsStatic(b)) cpBodyActivate(a);
		if(cpBodyIsRogue(a) && !cpBodyIsStatic(a)) cpBodyActivate(b);
	}
	
	// Generate components and deactivate sleeping ones
	for(int i=0; i<bodies->num;){
		cpBody *body = (cpBody*)bodies->arr[i];
		
		if(ComponentRoot(body) == NULL){
			// Body not in a component yet. Perform a DFS to flood fill mark 
			// the component in the contact graph using this body as the root.
			FloodFillComponent(body, body);
			
			// Check if the component should be put to sleep.
			if(!ComponentActive(body, space->sleepTimeThreshold)){
				cpArrayPush(space->sleepingComponents, body);
				CP_BODY_FOREACH_COMPONENT(body, other) cpSpaceDeactivateBody(space, other);
				
				// cpSpaceDeactivateBody() removed the current body from the list.
				// Skip incrementing the index counter.
				continue;
			}
		}
		
		i++;
		
		// Only sleeping bodies retain their component node pointers.
		body->node.root = NULL;
		body->node.next = NULL;
	}
}

void
cpBodySleep(cpBody *body)
{
	cpBodySleepWithGroup(body, NULL);
}

void
cpBodySleepWithGroup(cpBody *body, cpBody *group){
	cpAssert(!cpBodyIsStatic(body) && !cpBodyIsRogue(body), "Rogue and static bodies cannot be put to sleep.");
	
	cpSpace *space = body->space;
	cpAssert(space, "Cannot put a rogue body to sleep.");
	cpAssert(!space->locked, "Bodies cannot be put to sleep during a query or a call to cpSpaceStep(). Put these calls into a post-step callback.");
	cpAssert(group == NULL || cpBodyIsSleeping(group), "Cannot use a non-sleeping body as a group identifier.");
	
	if(cpBodyIsSleeping(body)){
		cpAssert(ComponentRoot(body) == ComponentRoot(group), "The body is already sleeping and it's group cannot be reassigned.");
		return;
	}
	
	CP_BODY_FOREACH_SHAPE(body, shape) cpShapeUpdate(shape, body->p, body->rot);
	cpSpaceDeactivateBody(space, body);
	
	if(group){
		cpBody *root = ComponentRoot(group);
		
		cpComponentNode node = {root, root->node.next, 0.0f};
		body->node = node;
		
		root->node.next = body;
	} else {
		cpComponentNode node = {body, NULL, 0.0f};
		body->node = node;
		
		cpArrayPush(space->sleepingComponents, body);
	}
	
	cpArrayDeleteObj(space->bodies, body);
}

static void
activateTouchingHelper(cpShape *shape, cpContactPointSet *points, cpArray **bodies){
	cpBodyActivate(shape->body);
}

void
cpSpaceActivateShapesTouchingShape(cpSpace *space, cpShape *shape){
	cpArray *bodies = NULL;
	cpSpaceShapeQuery(space, shape, (cpSpaceShapeQueryFunc)activateTouchingHelper, &bodies);
}
