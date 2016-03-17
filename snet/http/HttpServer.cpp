//
//  HttpServer.cpp
//  snet
//
//  Created by San on 16/3/2.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#include "HttpServer.h"

MUTEX_T HttpServer::s_mtx;
SendList_t HttpServer::s_sendList;

void HttpTask::run() {
    HttpResponse res;
    if(m_callback)
        m_callback(m_method, m_url, &res);
    else
        res.setHttp404Status();
    res.setAlive(false);
    HttpServer::addResponseList(m_fd, res);
}

HttpServer::HttpServer(IOLoop* loop):m_loop(loop) {
    m_svr = std::make_shared<TcpStream>(m_loop);
}

HttpServer::~HttpServer() {
    delete m_loop;
}

int HttpServer::start(const char* ip, int port, int task_count) {
    g_httpThreadPool.init(task_count);
    m_svr->async_listening(ip, port, std::bind(&HttpServer::onConn, this, std::placeholders::_1));
    m_svr->async_read(std::bind(&HttpServer::onRead, this, std::placeholders::_1, std::placeholders::_2));
    m_loop->add_handle(std::bind(&HttpServer::runInLoop, this, std::placeholders::_1));
    return 0;
}

void HttpServer::setHttpCallback(const HttpCallback_t& callback) {
    m_httpCallback = callback;
}

void HttpServer::addLoopFunc(IOLoop::Function_t func) {
    m_loop->add_handle(func);
}

void HttpServer::addResponseList(int fd, const HttpResponse& res) {
    MUTEXGUARD_T lck(s_mtx);
    s_sendList.insert(std::make_pair(fd, res));
}

void HttpServer::sendResponseList() {
    SendList_t sendlist;
    {
        MUTEXGUARD_T lck(s_mtx);
        if (!s_sendList.empty()) {
            sendlist.swap(s_sendList);
        }
    }
    for (auto& obj : sendlist) {
        StreamPtr_t stream = FindConnectedStream(obj.first);
        if (stream != nullptr) {
            std::string data = obj.second.packet();
            stream->async_write(data.c_str(), (int)data.length());
        }
    }
}

void HttpServer::runInLoop(void* arg) {
    sendResponseList();
    m_loop->add_handle(std::bind(&HttpServer::runInLoop, this, std::placeholders::_1));
}

void HttpServer::onConn(const StreamPtr_t& stream) {
    stream->setWritenCallback(std::bind(&HttpServer::onWriten, this, std::placeholders::_1));
}

void HttpServer::onRead(const StreamPtr_t& stream, Buffer* buf) {
    HttpParserObj* obj = HttpParserObj::GetInstance();
    obj->parseBuffer(buf->buffer(), buf->offset());
//    LOG_STDOUT("Request: %s", buf);
    
    if (obj->ready()) {
        LOG_STDOUT("Url: %s", obj->url().c_str());
        buf->read(NULL, obj->length());
        
//        HttpResponse res;
//        res.setHttp404Status();
//        HttpServer::addResponseList(stream->fd(), res);
        
        HttpTask* tsk = new HttpTask(stream->fd(),obj->method(), obj->url());
        tsk->setHttpCallback(m_httpCallback);
        g_httpThreadPool.addTask(tsk);
    }
}

void HttpServer::onWriten(const StreamPtr_t& stream) {
    stream->async_close();
}