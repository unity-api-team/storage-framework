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

#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/Handler.h>

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

RootImpl::RootImpl(storage::internal::ItemMetadata const& md, weak_ptr<Account> const& account)
    : ItemBase(md.item_id, ItemType::root)
    , FolderBase(md.item_id, ItemType::root)
    , RootBase(md.item_id, account)
    , ItemImpl(md, ItemType::root)
    , FolderImpl(md, ItemType::root)
{
}

QFuture<QVector<Folder::SPtr>> RootImpl::parents() const
{
    if (!get_root())
    {
        return make_exceptional_future<QVector<Folder::SPtr>>(RuntimeDestroyedException("Root::parents()"));
    }
    return make_ready_future(QVector<Folder::SPtr>());  // For the root, we return an empty vector.
}

QVector<QString> RootImpl::parent_ids() const
{
    if (!get_root())
    {
        return make_exceptional_future<QVector<QString>>(RuntimeDestroyedException("Root::parent_ids()"));
    }
    return QVector<QString>();  // For the root, we return an empty vector.
}

QFuture<void> RootImpl::delete_item()
{
    if (!get_root())
    {
        return make_exceptional_future(RuntimeDestroyedException("Root::delete_item()"));
    }
    // Cannot delete root.
    return make_exceptional_future(LogicException("Root::delete_item(): root item cannot be deleted"));
}

QFuture<int64_t> RootImpl::free_space_bytes() const
{
    if (!get_root())
    {
        return make_exceptional_future<int64_t>(RuntimeDestroyedException("Root::free_space_bytes()"));
    }
    // TODO, need to refresh metadata here instead.
    return make_ready_future(int64_t(1));
}

QFuture<int64_t> RootImpl::used_space_bytes() const
{
    if (!get_root())
    {
        return make_exceptional_future<int64_t>(RuntimeDestroyedException("Root::used_space_bytes()"));
    }
    // TODO, need to refresh metadata here instead.
    return make_ready_future(int64_t(1));
}

QFuture<Item::SPtr> RootImpl::get(QString native_identity) const
{
    if (!get_root())
    {
        return make_exceptional_future<Item::SPtr>(RuntimeDestroyedException("Root::get()"));
    }

    auto prov = provider();
    auto reply = prov->Metadata(native_identity);

    auto process_reply = [this](decltype(reply) const& reply, QFutureInterface<Item::SPtr>& qf)
    {
        shared_ptr<Account> acc;
        try
        {
            acc = account();
        }
        catch (RuntimeDestroyedException const&)
        {
            make_exceptional_future<Item::SPtr>(qf, RuntimeDestroyedException("Root::get()"));
            return;
        }

        auto md = reply.value();
        Item::SPtr item;
        if (md.type == ItemType::root)
        {
            item = make_root(md, acc);
        }
        else
        {
            // acc owns the root, so the root weak_ptr is guaranteed to be lockable.
            item = ItemImpl::make_item(md, root_);
        }
        make_ready_future(qf, item);
    };

    auto handler = new Handler<Item::SPtr>(const_cast<RootImpl*>(this), reply, process_reply);
    return handler->future();
}

Root::SPtr RootImpl::make_root(storage::internal::ItemMetadata const& md, weak_ptr<Account> const& account)
{
    assert(md.type == ItemType::root);
    assert(account.lock());

    auto impl = new RootImpl(md, account);
    Root::SPtr root(new Root(impl));
    impl->set_root(root);
    impl->set_public_instance(root);
    return root;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
