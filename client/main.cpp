#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>
#include "argParse.h"
#include "logger.h"
#define LERR SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::ERR)
#define LINFO SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::INFO)
#define LMSG SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::NONE)


void sendMsg(int id, std::string cmdName, int payload=0) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        LERR << "Display not init";
        exit(-1);
    }
    const int screen = DefaultScreen(display);
	Window rootWin = RootWindow(display, screen);

    XEvent msg;
    memset(&msg, 0, sizeof(msg));
    msg.xclient.type = ClientMessage;
    msg.xclient.message_type = XInternAtom(display, "UNKNOWWM_CLIENT_EVENT", False);
    msg.xclient.window = rootWin;
    msg.xclient.format = 32;
    msg.xclient.data.l[0] = id;
    msg.xclient.data.l[1] = payload;
    XSendEvent(display, rootWin, False, SubstructureRedirectMask, &msg);
    if (display) {
        XCloseDisplay(display);
    }
}

int main(int argc, char *args[]) {
    if (argc == 1) {
        LERR << "Must contains 1 or 2 input commands";
        return -1;
    }

    int id = 0;
    ap::ArgParse ap(L"UNKNOWWM Client");
    {
        std::wstring cmd = L"change_desktop";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_INT, [&ap, &cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd + L" <int>");
        id++;
    }
    {
        std::wstring cmd = L"change_monitor";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_INT, [&ap, &cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd + L" <int>");
        id++;
    }
    {
        std::wstring cmd = L"switch_mode";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_INT, [&ap, &cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd + L" <int> \n0 - V stack left \n1 - V stack right \n2 - H stack up \n3 - H stack down \n4 - monocle \n5 - grid \n6 - float \n7 - fibbonaci \n8 - double stack");
        id++;
    }
    {
        std::wstring cmd = L"quit";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"toggle_panel";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"next_win";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"prev_win";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"next_decktop";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"prev_decktop";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"toggle_fullscreen";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"toggle_float";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"next_layout";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"prev_layout";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"restart";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }
    {
        std::wstring cmd = L"restart_monitors";
        ap.addArg(std::vector<std::wstring>{cmd}, ap::PayloadType::TYPE_BOOL, [&ap, cmd, id]() {
            auto res = ap.payloadMap.find(cmd);
            sendMsg(id, std::string(cmd.begin(), cmd.end()), (*(*res).second).intVal);
        }, nullptr, cmd);
        id++;
    }

    ap.parse(argc, args);
    return 0;
}
