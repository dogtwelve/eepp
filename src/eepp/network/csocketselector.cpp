#include <eepp/network/csocketselector.hpp>
#include <eepp/network/csocket.hpp>
#include <eepp/network/platform/platformimpl.hpp>
#include <utility>

#ifdef _MSC_VER
	#pragma warning(disable : 4127) // "conditional expression is constant" generated by the FD_SET macro
#endif

namespace EE { namespace Network {

struct cSocketSelector::cSocketSelectorImpl {
	fd_set AllSockets;   ///< Set containing all the sockets handles
	fd_set SocketsReady; ///< Set containing handles of the sockets that are ready
	int	MaxSocket;	///< Maximum socket handle
};

cSocketSelector::cSocketSelector() :
	mImpl( eeNew( cSocketSelectorImpl, () ) )
{
	Clear();
}

cSocketSelector::cSocketSelector(const cSocketSelector& copy) :
	mImpl( eeNew( cSocketSelectorImpl, (*copy.mImpl) ) )
{
}

cSocketSelector::~cSocketSelector() {
	eeSAFE_DELETE( mImpl );
}

void cSocketSelector::Add(cSocket& socket) {
	SocketHandle handle = socket.GetHandle();

	if (handle != Private::cSocketImpl::InvalidSocket()) {
		FD_SET(handle, &mImpl->AllSockets);

		int size = static_cast<int>(handle);
		if (size > mImpl->MaxSocket)
			mImpl->MaxSocket = size;
	}
}

void cSocketSelector::Remove(cSocket& socket) {
	FD_CLR(socket.GetHandle(), &mImpl->AllSockets);
	FD_CLR(socket.GetHandle(), &mImpl->SocketsReady);
}

void cSocketSelector::Clear() {
	FD_ZERO(&mImpl->AllSockets);
	FD_ZERO(&mImpl->SocketsReady);

	mImpl->MaxSocket = 0;
}

bool cSocketSelector::Wait(cTime timeout) {
	// Setup the timeout
	timeval time;
	time.tv_sec  = static_cast<long>(timeout.AsMicroseconds() / 1000000);
	time.tv_usec = static_cast<long>(timeout.AsMicroseconds() % 1000000);

	// Initialize the set that will contain the sockets that are ready
	mImpl->SocketsReady = mImpl->AllSockets;

	// Wait until one of the sockets is ready for reading, or timeout is reached
	int count = select(mImpl->MaxSocket + 1, &mImpl->SocketsReady, NULL, NULL, timeout != cTime::Zero ? &time : NULL);

	return count > 0;
}

bool cSocketSelector::IsReady(cSocket& socket) const {
	return FD_ISSET(socket.GetHandle(), &mImpl->SocketsReady) != 0;
}

cSocketSelector& cSocketSelector::operator =(const cSocketSelector& right) {
	cSocketSelector temp(right);

	std::swap(mImpl, temp.mImpl);

	return *this;
}

}}
