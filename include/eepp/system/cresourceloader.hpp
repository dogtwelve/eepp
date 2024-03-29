#ifndef EE_SYSTEMCRESOURCELOADER
#define EE_SYSTEMCRESOURCELOADER

#include <eepp/system/base.hpp>
#include <eepp/system/cobjectloader.hpp>

namespace EE { namespace System {

#define THREADS_AUTO (eeINDEX_NOT_FOUND)

/** @brief A simple resource loader that can load a batch of resources synchronously or asynchronously */
class EE_API cResourceLoader {
	public:
		typedef cb::Callback1<void, cResourceLoader *> ResLoadCallback;

		/** @param MaxThreads Set the maximun simultaneous threads to load resources, THREADS_AUTO will use the cpu number of cores. */
		cResourceLoader( const Uint32& MaxThreads = THREADS_AUTO );

		virtual ~cResourceLoader();

		/** @brief Adds a resource to load.
		**	Must be called before the loading starts.
		**	Once an object loader is added to the resource loader, the instance of that object will be managed and released by the loader.
		**	@param Object The instance object loader to load
		*/
		void			Add( cObjectLoader * Object );

		/** @brief Starts loading the resources.
		**	@param Cb A callback that is called when the resources finished loading. */
		void 			Load( ResLoadCallback Cb );

		/** @brief Starts loading the resources. */
		void 			Load();

		/** @brief Unload all the resources already loaded. */
		void			Unload();

		/** @brief Update must be called from the thread that started the loading to update the state of the resource loader. */
		virtual void 	Update();

		/** @returns If the resources were loaded. */
		virtual bool	IsLoaded();

		/** @returns If the resources are still loading. */
		virtual bool	IsLoading();

		/** @returns If the resource loader is asynchronous */
		bool			Threaded() const;

		/** @brief Sets if the resource loader is asynchronous.
		**	This must be called before the load starts. */
		void			Threaded( const bool& threaded );

		/** @brief Clears the resources added to load that werent loaded, and delete the instances of the loaders.
		**	@param ClearObjectsLoaded Sets if the objects loader that were already loaded must be also deleted ( it will not unload the loaded resources, but the instance of the object loader ). */
		bool			Clear( const bool& ClearObjectsLoaded = true );

		/** @return The aproximate percent of progress ( between 0 and 100 ) */
		eeFloat			Progress();

		/** @returns The number of resources added to load. */
		Uint32			Count() const;
	protected:
		bool			mLoaded;
		bool			mLoading;
		bool			mThreaded;
		Uint32			mThreads;

		std::list<ResLoadCallback>	mLoadCbs;
		std::list<cObjectLoader *>	mObjs;
		std::list<cObjectLoader *>	mObjsLoaded;

		void			SetThreads();

		virtual void	SetLoaded();
};

}}

#endif
