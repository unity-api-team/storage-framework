#include <unity/storage/qt/client/internal/remote_client/UploaderImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/internal/remote_client/CancelUploadHandler.h>
#include <unity/storage/qt/client/internal/remote_client/FinishUploadHandler.h>
#include <unity/storage/qt/client/Uploader.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

UploaderImpl::UploaderImpl(QString upload_id,
                           int fd,
                           int64_t size,
                           QString const& old_etag,
                           weak_ptr<Root> root,
                           ProviderInterface& provider)
    : UploaderBase(old_etag == "" ? ConflictPolicy::overwrite : ConflictPolicy::error_if_conflict, size)
    , upload_id_(upload_id)
    , old_etag_(old_etag)
    , root_(root)
    , provider_(provider)
    , write_socket_(new QLocalSocket)
{
    assert(!upload_id.isEmpty());
    assert(root_.lock());
    assert(fd >= 0);
    write_socket_->setSocketDescriptor(fd, QLocalSocket::ConnectedState, QIODevice::WriteOnly);
}

UploaderImpl::~UploaderImpl()
{
    cancel();
}

shared_ptr<QLocalSocket> UploaderImpl::socket() const
{
    return write_socket_;
}

QFuture<shared_ptr<File>> UploaderImpl::finish_upload()
{
    auto handler = new FinishUploadHandler(provider_.FinishUpload(upload_id_), root_);
    return handler->future();
}

QFuture<void> UploaderImpl::cancel() noexcept
{
    auto handler = new CancelUploadHandler(provider_.CancelUpload(upload_id_));
    return handler->future();
}

Uploader::SPtr UploaderImpl::make_uploader(QString const& upload_id,
                                           int fd,
                                           int64_t size,
                                           QString const& old_etag,
                                           weak_ptr<Root> root,
                                           ProviderInterface& provider)
{
    auto impl = new UploaderImpl(upload_id, fd, size, old_etag, root, provider);
    Uploader::SPtr uploader(new Uploader(impl));
    return uploader;
}

}  // namespace remote_client
}  // namespace intternal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "UploaderImpl.moc"
