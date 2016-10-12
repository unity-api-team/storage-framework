/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/storage/qt/Uploader.h>

#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>

namespace unity
{
namespace storage
{
namespace internal
{

class ItemMetadata;

}  // namespace internal

namespace qt
{
namespace internal
{

class UploaderImpl : public QObject
{
    Q_OBJECT
public:
    UploaderImpl(std::shared_ptr<ItemImpl> const& item_impl,
                 QString const& method,
                 QDBusPendingReply<QString, QDBusUnixFileDescriptor>& reply,
                 std::function<void(storage::internal::ItemMetadata const&)> const& validate,
                 Item::ConflictPolicy policy,
                 qint64 size_in_bytes);
    UploaderImpl(StorageError const& e);
    virtual ~UploaderImpl();

    bool isValid() const;
    Uploader::Status status() const;
    StorageError error() const;
    Item::ConflictPolicy policy() const;
    qint64 sizeInBytes() const;
    Item item() const;

    void finishUpload();
    void cancel();

    static Uploader* make_job(std::shared_ptr<ItemImpl> const& item_impl,
                              QString const& method,
                              QDBusPendingReply<QString, QDBusUnixFileDescriptor>& reply,
                              std::function<void(storage::internal::ItemMetadata const&)> const& validate,
                              Item::ConflictPolicy policy,
                              qint64 size_in_bytes);
    static Uploader* make_job(StorageError const& e);

private:
    Uploader* public_instance_;
    Uploader::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<ItemImpl> item_impl_;
    std::function<void(storage::internal::ItemMetadata const&)> validate_;
    Item::ConflictPolicy policy_ = Item::ConflictPolicy::Overwrite;
    qint64 size_in_bytes_ = 0;
    QString upload_id_;
    QDBusUnixFileDescriptor fd_;
    bool finalizing_ = false;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
