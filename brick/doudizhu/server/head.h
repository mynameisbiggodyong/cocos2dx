#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <map>

using namespace std;

#define BUFFER_SIZE 2048
class GameRoom;
class Player;
extern map<string, GameRoom *> roomMap;
extern map<string, Player*> playerMap;
extern map<int, Player*> socketMap;
enum RoomFlag
{
	LEGAL = 0,
	NONE,
	EMPTY,
	FULL,
};
enum PokerType
{
	FANGKUAI = 1,
	MEIHUA,
	HONGXIN,
	HEITAO,
	DAXIAOWANG
};
enum PlayerType
{
	UNDEFINED,
	DIZHU = 1,
	NONGMING,
};
enum MessageType
{
	ILLEGAL = 0,
	GET_ROOM_LIST = 1,	//获取房间列表
	GET_ROOM_LIST_RESULT,
	CREATE_ROOM,		//创建房间同时创建用户
	CREATE_ROOM_RESULT,
	ENTER_ROOM,		//进入房间同时创建用户
	ENTER_ROOM_RESULT,
	ENTER_ROOM_OTHERS,	//别人进入房间
	READY,			//准备游戏
	READY_RESULT,
	READY_OTHERS,		//别人准备
	OUT_CARD,		//自己出牌
	OUT_CARD_RESULT,
	OUT_CARD_OTHERS,	//别人出牌
	START_GAME,		//开始游戏只能够从服务器发出
	END_GAME,		//游戏结束

};

/** 客户端发送的消息格式
 * 长度	1字节
 * 类型	1字节
 * 	获取房间列表：
 * 		NONE
 * 	创建房间
 * 		房间名称
 * 	进入房间：
 *		房间名称
 * 	准备游戏：
 *		NONE
 * 	出牌：
 * 		牌列表，每张牌占用一个字节
 *
 *
 **/

/** 服务器发送的消息格式
 * 长度	1字节
 * 类型	1字节
 * 	获取房间列表：
 * 		房间名称 空格分割
 * 	创建房间：
 * 		失败OR成功
 * 	进入房间：
 * 		其它玩家的名称 空格 准备状态 换行符
 * 		...
 * 	别人进入房间：
 *		玩家名称
 * 	准备游戏：
 * 		NONE
 * 	别人准备游戏：
 * 		玩家名称
 * 	出牌：
 * 		成功或是失败 空格 牌列表：每张占用一个字节
 * 	别人出牌：
 * 		玩家名称 空格 牌列表
 * 	开始游戏：
 * 		牌列表
 * 	游戏结束：
 * 		玩家 是否是赢家 剩余的牌列表 换行符
 *		...
 **/

class PokerCard
{
public:
	bool isQueen()
	{
		if(this->valueInAll >= 52)
		{
			return true;
		}
		return false;
	}
	int	type;
	int	valueInType;
	int	valueInAll;
};
class Player
{
public:
	int	identity;		//身份
	int	ready;			//是否准备
	string 	playerName;
	string	roomName;
	char	readBuf[BUFFER_SIZE];	//每个玩家都有一个消息缓冲区
	int	readBufLen;
	int	fd;			//对应的socket描述符
	int 	remain_length;

	GameRoom* 	joinGame(string roomName);
	int 		exitGame();
};

class GameRoom
{
public:
	string 	roomName;
	Player*	players[3];
	int 	currentPlayers;
	int	isRunning;
	GameRoom();
	GameRoom(string name);
};

Player* addPlayer(string name);
void deletePlayer(Player* player);
void sendRoomList(int fd);
void handle_message(int fd);			//处理用户消息
void handle_get_room_list(Player *player);	//获取房间列表处理函数
void handle_create_room(Player* player);	//创建房间处理函数
void handle_enter_room(Player *player);		//进入房间处理函数
void handle_ready(Player *player);		//玩家准备处理函数
void handle_out_card(Player *player);		//玩家出牌处理函数