#include "util/common.h"
#include "model/struct.h"
#include "util/global.h"
#include "util/tools.h"

std::string latest_cout;

void cout_dupl(std::string str, bool endl)
{
    if (latest_cout != str)
    {
        std::cout << str;
        if (endl)
            std::cout << std::endl;
        latest_cout = str;
    }
}

void handleWebsocket(std::string response, SharedState &state)
{
    try
    {
        json parsed = json::parse(response);

        std::string type = parsed["type"];
        if (type == "login-res")
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.user.ticket = parsed["data"]["ticket"];
            std::cout << "아래 링크로 접속해 로그인하세요!" << std::endl;
            std::cout << parsed["data"]["url"] << std::endl;
        }
        else if (type == "ticket-check-res")
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            std::lock_guard<std::mutex> lockpt(ptstate.mutex);
            if (parsed["data"].contains("message"))
            {
                if (parsed["data"]["message"] != "unauth-ticket")
                    cout_dupl(parsed["data"]["message"], true);
                return;
            }
            state.isLoggedIn = true;
            state.user.id = parsed["data"]["id"];
            state.user.email = parsed["data"]["email"];
            state.user.name = parsed["data"]["name"];
            state.user.nickname = parsed["data"]["nickname"];
            state.user.picture = parsed["data"]["picture"];

            for (auto &r : parsed["data"]["rooms"])
            {
                RoomState room;
                room.roomId = r["_id"];
                room.roomName = r["roomName"];
                room.description = r["description"];
                room.latestChat = r["latestChat"];
                // for (auto &c : r["chats"])
                // {
                //     Chat chat;
                //     chat.userId = c["userId"];
                //     chat.text = c["text"];
                //     chat.createdAt = c["createdAt"];
                //     room.chats.push_back(chat);
                // }
                for (auto &p : r["participants"])
                {
                    Participant pt;
                    pt.id = p["userId"]["_id"];
                    pt.email = p["userId"]["email"];
                    pt.latest_access = p["userId"]["latest_access"];
                    pt.name = p["userId"]["name"];
                    pt.nickname = p["userId"]["nickname"];
                    pt.picture = p["userId"]["picture"];
                    ptstate.participants[pt.id] = pt;
                    room.participants.insert({pt.id, p["lastReadAt"]});
                }

                state.rooms[room.roomId] = room;
            };
        }
        else if (type == "create-room-res" || type == "join-room-res")
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            std::lock_guard<std::mutex> lock_join(join_room.mutex);

            RoomState room;
            room.roomName = parsed["data"]["room"]["roomName"];
            room.roomId = parsed["data"]["room"]["_id"];
            room.description = parsed["data"]["room"]["description"];
            room.latestChat = parsed["data"]["room"]["latestChat"];
            // for (auto &p : parsed["data"]["room"]["participants"])
            // {
            //     Participant pt;
            //     pt.id = p["_id"];
            //     pt.name = p["name"];
            //     pt.nickname = p["nickname"];
            //     room.participants.push_back(pt);
            // }
            // room.roomName = roomName;

            state.rooms[room.roomId] = room;
            if (type.starts_with("create"))
                std::cout << "방이 성공적으로 생성되었습니다!\n방 이름: " << room.roomName << std::endl;
            else
                join_room.updated = true;
        }
        else if (type == "send-message-res")
        {
            Chat ch;
            ch.userId = parsed["data"]["chat"]["userId"];
            ch.text = parsed["data"]["chat"]["text"];
            ch.createdAt = parsed["data"]["chat"]["createdAt"];
            std::string roomId = parsed["data"]["chat"]["roomId"];

            std::lock_guard<std::mutex> lock(state.mutex);

            // RoomState r;
            // for (auto &rm : state.rooms)
            // {
            //     if (rm.second.roomId == roomId)
            //     {
            //         r = rm.second;
            //         break;
            //     }
            // }
            std::string p;
            for (auto &pt : state.rooms[roomId].participants)
            {
                if (pt.first == ch.userId)
                {
                    p = pt.first;
                    break;
                }
            }

            state.rooms[roomId].chats.push_back(ch);

            if (state.currentRoom == nullptr || state.currentRoom->roomId != roomId)
            {

                std::lock_guard<std::mutex> lock(ptstate.mutex);
                p = ptstate.participants[p].nickname;

                std::string msg = std::format("{}에서 {}의 새 메시지: {}", state.rooms[roomId].roomName, p, ch.text);
                std::string title = "새로운 메시지 알림";

                openMessageBox(title, msg);
            }
        }
        else if (type == "get-room-list-res")
        {
            std::lock_guard<std::mutex> lock(visiting.mutex);
            for (auto &p : parsed["data"]["rooms"])
            {
                VisitingRoom vr;
                vr.roomId = p["_id"];
                vr.roomName = p["roomName"];
                vr.description = p["description"];
                vr.latestChat = p["latestChat"];
                visiting.rooms.push_back(vr);
                visiting.updated = true;
            }
        }
        else if (type == "get-chat-list-res")
        {
            std::lock_guard<std::mutex> lockchat(load_chat.mutex);
            std::lock_guard<std::mutex> lockshrd(shared.mutex);
            std::lock_guard<std::mutex> lockpt(ptstate.mutex);
            shared.currentRoom->chats.clear();

            for (auto &p : parsed["data"]["chats"])
            {
                Chat c;
                c.userId = p["userId"];
                c.createdAt = p["createdAt"];
                c.text = p["text"];
                shared.currentRoom->chats.push_back(c);
            }

            for (auto &p : parsed["data"]["participants"])
            {
                Participant pt;
                pt.id = p["_id"];
                pt.email = p["email"];
                pt.latest_access = p["latest_access"];
                pt.name = p["name"];
                pt.nickname = p["nickname"];
                pt.picture = p["picture"];
                ptstate.participants[pt.id] = pt;
                shared.currentRoom->participants[pt.id] = p["lastReadAt"];
            }
            load_chat.updated = true;
        }
        else if (type == "read-chat-event")
        {
            // 서버단 신뢰
            // roomId, userId, timestamp
            std::lock_guard<std::mutex> lockpt(shared.mutex);
            shared.rooms[parsed["data"]["roomId"]].participants[parsed["data"]["userId"]] = parsed["data"]["timstamp"];
        }
        else
        {
            std::cout << "미확인 타입 " << type << std::endl;
        }
    }
    catch (...)
    {
        std::cerr << "[JSON 파싱 실패] " << response << std::endl;
    }
}