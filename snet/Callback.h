//
//  Callback.h
//  snet
//
//  Created by San on 16/2/25.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef __snet__Callback_h
#define __snet__Callback_h

#include "stdafx.h"

class IOLoop;
class IOEvent;
class IOStream;
class Buffer;
//class IPAddress;
typedef std::shared_ptr<IOStream> StreamPtr_t;
typedef std::map<int, StreamPtr_t> StreamMap_t;

typedef std::function<void()> TimerCallback_t;
typedef std::function<void (const StreamPtr_t&)> ConnectionCallback_t;
typedef std::function<void (const IPAddress&)> CloseCallback_t;
typedef std::function<void (const IPAddress&, void*)> ErrorCallback_t;
typedef std::function<void (const StreamPtr_t&)> WriteCompleteCallback_t;
//typedef std::function<void (const StreamPtr_t&, const char*,int)> ReadMessageCallback_t;
typedef std::function<void (const StreamPtr_t&, Buffer*)> ReadMessageCallback_t;
//typedef std::function<void (const StreamPtr_t&, size_t)> HighWaterMarkCallback_t;

#endif /* __snet__Callback_h */
