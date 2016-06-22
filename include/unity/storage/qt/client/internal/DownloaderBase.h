#pragma once

#include <unity/storage/common.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <memory>

class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class File;

namespace internal
{

class DownloaderBase : public QObject
{
    Q_OBJECT

public:
    DownloaderBase(std::weak_ptr<File> file);
    DownloaderBase(DownloaderBase const&) = delete;
    DownloaderBase& operator=(DownloaderBase const&) = delete;

    virtual std::shared_ptr<File> file() const = 0;
    virtual std::shared_ptr<QLocalSocket> socket() const = 0;
    virtual QFuture<TransferState> finish_download() = 0;
    virtual QFuture<void> cancel() noexcept = 0;

protected:
    std::shared_ptr<File> file_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
