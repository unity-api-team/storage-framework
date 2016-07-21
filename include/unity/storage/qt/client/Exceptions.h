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

#include <unity/storage/visibility.h>

#include <QException>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

/**
\brief Base exception class for all exceptions returned by the API.
*/
class UNITY_STORAGE_EXPORT StorageException : public QException
{
public:
    StorageException();
    ~StorageException();

    virtual StorageException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates errors in the communication with the storage provider service.
*/
class UNITY_STORAGE_EXPORT LocalCommsException : public StorageException
{
public:
    LocalCommsException();
    ~LocalCommsException();

    virtual LocalCommsException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates errors in the communication between the storage provider and the cloud service.
*/
class UNITY_STORAGE_EXPORT RemoteCommsException : public StorageException
{
public:
    RemoteCommsException();
    ~RemoteCommsException();

    virtual RemoteCommsException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that the caller invoked an operation on a file or folder that was deleted.
*/
class UNITY_STORAGE_EXPORT DeletedException : public StorageException
{
public:
    DeletedException();
    ~DeletedException();

    virtual DeletedException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that the caller destroyed the runtime.
*/
class UNITY_STORAGE_EXPORT RuntimeDestroyedException : public StorageException
{
public:
    RuntimeDestroyedException();
    ~RuntimeDestroyedException();

    virtual RuntimeDestroyedException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that an item does not exist.
*/
class UNITY_STORAGE_EXPORT NotExistException : public StorageException
{
public:
    NotExistException();
    ~NotExistException();

    virtual NotExistException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that an upload detected a version mismatch.
*/
class UNITY_STORAGE_EXPORT ConflictException : public StorageException
{
public:
    ConflictException();
    ~ConflictException();

    virtual ConflictException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that an upload or download was cancelled before it could complete.
*/
class UNITY_STORAGE_EXPORT CancelledException : public StorageException
{
public:
    CancelledException();
    ~CancelledException();

    virtual CancelledException* clone() const override;
    virtual void raise() const override;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
