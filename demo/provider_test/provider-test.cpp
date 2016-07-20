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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/TempfileUploadJob.h>
#include <unity/storage/provider/UploadJob.h>

#include <unistd.h>

using namespace unity::storage;
using namespace unity::storage::provider;
using namespace std;

class MyProvider : public ProviderBase
{
public:
    MyProvider();

    boost::future<ItemList> roots(Context const& ctx) override;
    boost::future<tuple<ItemList,string>> list(
        string const& item_id, string const& page_token,
        Context const& ctx) override;
    boost::future<ItemList> lookup(
        string const& parent_id, string const& name,
        Context const& ctx) override;
    boost::future<Item> metadata(
        string const& item_id, Context const& ctx) override;
    boost::future<Item> create_folder(
        string const& parent_id, string const& name,
        Context const& ctx) override;

    boost::future<unique_ptr<UploadJob>> create_file(
        string const& parent_id, string const& title,
        string const& content_type, bool allow_overwrite,
        Context const& ctx) override;
    boost::future<unique_ptr<UploadJob>> update(
        string const& item_id, string const& old_etag,
        Context const& ctx) override;

    boost::future<unique_ptr<DownloadJob>> download(
        string const& item_id, Context const& ctx) override;

    boost::future<void> delete_item(
        string const& item_id, Context const& ctx) override;
    boost::future<Item> move(
        string const& item_id, string const& new_parent_id,
        string const& new_name, Context const& ctx) override;
    boost::future<Item> copy(
        string const& item_id, string const& new_parent_id,
        string const& new_name, Context const& ctx) override;
};

class MyUploadJob : public TempfileUploadJob
{
public:
    using TempfileUploadJob::TempfileUploadJob;

    boost::future<void> cancel() override;
    boost::future<Item> finish() override;
};

class MyDownloadJob : public DownloadJob
{
public:
    using DownloadJob::DownloadJob;

    boost::future<void> cancel() override;
    boost::future<void> finish() override;
};

MyProvider::MyProvider()
{
}

boost::future<ItemList> MyProvider::roots(Context const& ctx)
{
    printf("roots() called by %s (%d)\n", ctx.security_label.c_str(), ctx.pid);
    ItemList roots = {
        {"root_id", "", "Root", "etag", ItemType::root, {}},
    };
    return boost::make_ready_future<ItemList>(roots);
}

boost::future<tuple<ItemList,string>> MyProvider::list(
    string const& item_id, string const& page_token,
    Context const& ctx)
{
    printf("list('%s', '%s') called by %s (%d)\n", item_id.c_str(), page_token.c_str(), ctx.security_label.c_str(), ctx.pid);
    if (item_id != "root_id")
    {
        return boost::make_exceptional_future<tuple<ItemList,string>>(runtime_error("unknown folder"));
    }
    if (page_token != "")
    {
        return boost::make_exceptional_future<tuple<ItemList,string>>(runtime_error("unknown page token"));
    }
    ItemList children = {
        {"child_id", "root_id", "Child", "etag", ItemType::file, {}}
    };
    boost::promise<tuple<ItemList,string>> p;
    p.set_value(make_tuple(children, string()));
    return p.get_future();
}

boost::future<ItemList> MyProvider::lookup(
    string const& parent_id, string const& name, Context const& ctx)
{
    printf("lookup('%s', '%s') called by %s (%d)\n", parent_id.c_str(), name.c_str(), ctx.security_label.c_str(), ctx.pid);
    if (parent_id != "root_id" || name != "Child")
    {
        return boost::make_exceptional_future<ItemList>(runtime_error("file not found"));
    }
    ItemList children = {
        {"child_id", "root_id", "Child", "etag", ItemType::file, {}}
    };
    return boost::make_ready_future<ItemList>(children);
}

boost::future<Item> MyProvider::metadata(string const& item_id,
                                         Context const& ctx)
{
    printf("metadata('%s') called by %s (%d)\n", item_id.c_str(), ctx.security_label.c_str(), ctx.pid);
    if (item_id == "root_id")
    {
        Item metadata{"root_id", "", "Root", "etag", ItemType::root, {}};
        return boost::make_ready_future<Item>(metadata);
    }
    else if (item_id == "child_id")
    {
        Item metadata{"child_id", "root_id", "Child", "etag", ItemType::file, {}};
        return boost::make_ready_future<Item>(metadata);
    }
    return boost::make_exceptional_future<Item>(runtime_error("no such file"));
}

boost::future<Item> MyProvider::create_folder(
    string const& parent_id, string const& name,
    Context const& ctx)
{
    printf("create_folder('%s', '%s') called by %s (%d)\n", parent_id.c_str(), name.c_str(), ctx.security_label.c_str(), ctx.pid);
    Item metadata{"new_folder_id", parent_id, name, "etag", ItemType::folder, {}};
    return boost::make_ready_future<Item>(metadata);
}

string make_job_id()
{
    static int last_job_id = 0;
    return to_string(++last_job_id);
}

boost::future<unique_ptr<UploadJob>> MyProvider::create_file(
    string const& parent_id, string const& title,
    string const& content_type, bool allow_overwrite,
    Context const& ctx)
{
    printf("create_file('%s', '%s', '%s', %d) called by %s (%d)\n", parent_id.c_str(), title.c_str(), content_type.c_str(), allow_overwrite, ctx.security_label.c_str(), ctx.pid);
    return boost::make_ready_future(unique_ptr<UploadJob>(new MyUploadJob(make_job_id())));
}

boost::future<unique_ptr<UploadJob>> MyProvider::update(
    string const& item_id, string const& old_etag, Context const& ctx)
{
    printf("update('%s', '%s') called by %s (%d)\n", item_id.c_str(), old_etag.c_str(), ctx.security_label.c_str(), ctx.pid);
    return boost::make_ready_future(unique_ptr<UploadJob>(new MyUploadJob(make_job_id())));
}

boost::future<unique_ptr<DownloadJob>> MyProvider::download(
    string const& item_id, Context const& ctx)
{
    printf("download('%s') called by %s (%d)\n", item_id.c_str(), ctx.security_label.c_str(), ctx.pid);

    unique_ptr<DownloadJob> job(new MyDownloadJob(make_job_id()));
    const char contents[] = "Hello world";
    write(job->write_socket(), contents, sizeof(contents));
    job->report_complete();
    return boost::make_ready_future(std::move(job));
}

boost::future<void> MyProvider::delete_item(
    string const& item_id, Context const& ctx)
{
    printf("delete('%s') called by %s (%d)\n", item_id.c_str(), ctx.security_label.c_str(), ctx.pid);
    return boost::make_ready_future();
}

boost::future<Item> MyProvider::move(
    string const& item_id, string const& new_parent_id,
    string const& new_name, Context const& ctx)
{
    printf("move('%s', '%s', '%s') called by %s (%d)\n", item_id.c_str(), new_parent_id.c_str(), new_name.c_str(), ctx.security_label.c_str(), ctx.pid);
    Item metadata{item_id, new_parent_id, new_name, "etag", ItemType::file, {}};
    return boost::make_ready_future(metadata);
}

boost::future<Item> MyProvider::copy(
    string const& item_id, string const& new_parent_id,
    string const& new_name, Context const& ctx)
{
    printf("copy('%s', '%s', '%s') called by %s (%d)\n", item_id.c_str(), new_parent_id.c_str(), new_name.c_str(), ctx.security_label.c_str(), ctx.pid);
    Item metadata{"new_item_id", new_parent_id, new_name, "etag", ItemType::file, {}};
    return boost::make_ready_future(metadata);
}

boost::future<void> MyUploadJob::cancel()
{
    printf("cancel_upload('%s')\n", upload_id().c_str());
    return boost::make_ready_future();
}

boost::future<Item> MyUploadJob::finish()
{
    printf("finish_upload('%s')\n", upload_id().c_str());

    string old_filename = file_name();
    string new_filename = upload_id() + ".txt";
    printf("Linking %s to %s\n", old_filename.c_str(), new_filename.c_str());
    unlink(new_filename.c_str());
    link(old_filename.c_str(), new_filename.c_str());

    Item metadata{"some_id", "", "some_upload", "etag", ItemType::file, {}};
    return boost::make_ready_future(metadata);
}

boost::future<void> MyDownloadJob::cancel()
{
    printf("cancel_download('%s')\n", download_id().c_str());
    return boost::make_ready_future();
}

boost::future<void> MyDownloadJob::finish()
{
    printf("finish_download('%s')\n", download_id().c_str());

    return boost::make_ready_future();}


int main(int argc, char **argv)
{
    Server<MyProvider> server("com.canonical.StorageFramework.Provider.ProviderTest", "google-drive-scope");
    server.init(argc, argv);
    server.run();
}