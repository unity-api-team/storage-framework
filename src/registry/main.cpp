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

#include "registryadaptor.h"
#include <unity/storage/internal/EnvVars.h>
#include <unity/storage/internal/InactivityTimer.h>
#include <unity/storage/internal/TraceMessageHandler.h>
#include <unity/storage/registry/internal/qdbus-last-error-msg.h>
#include <unity/storage/registry/internal/RegistryAdaptor.h>
#include <unity/storage/registry/Registry.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusError>
#include <QFileInfo>
#pragma GCC diagnostic pop

#if QT_VERSION < 0x050500
#define qInfo() qDebug()
#endif

using namespace unity::storage;
using namespace unity::storage::registry::internal;
using namespace std;

int main(int argc, char* argv[])
{
    auto const prog_name = QFileInfo(argv[0]).fileName();
    internal::TraceMessageHandler message_handler(prog_name);

    int rc = 1;
    try
    {
        QCoreApplication app(argc, argv);

        auto conn = QDBusConnection::sessionBus();

        int const timeout_ms = internal::EnvVars::registry_timeout_ms();
        auto timeout_func = [&app, timeout_ms]
        {
            qInfo().noquote().nospace() << "Exiting after " << QString::number(timeout_ms) << " ms of idle time";
            app.quit();
        };
        auto inactivity_timer = make_shared<unity::storage::internal::InactivityTimer>(timeout_ms, timeout_func);

        registry::internal::RegistryAdaptor registry_adaptor(conn, inactivity_timer);
        new ::RegistryAdaptor(&registry_adaptor);

        string const object_path = internal::EnvVars::registry_object_path();
        if (!conn.registerObject(object_path.c_str(), &registry_adaptor))
        {
            auto msg = last_error_msg(conn);
            throw runtime_error(string("Could not register object path ") + object_path + msg.toStdString());
        }

        qDBusRegisterMetaType<unity::storage::internal::AccountDetails>();
        qDBusRegisterMetaType<QList<unity::storage::internal::AccountDetails>>();

        string const bus_name = internal::EnvVars::registry_bus_name();
        if (!conn.registerService(bus_name.c_str()))
        {
            auto msg = last_error_msg(conn);
            throw runtime_error(string("Could not acquire DBus name ") + bus_name + msg.toStdString());
        }

        rc = app.exec();

        if (!conn.unregisterService(bus_name.c_str()))
        {
            auto msg = last_error_msg(conn);
            throw runtime_error(string("Could not release DBus name ") + bus_name + msg.toStdString());
        }
    }
    catch (std::exception const& e)
    {
        qCritical().noquote() << e.what();
    }

    return rc;
}
