constexpr int PORT_NUM = 4000;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 300;

constexpr int BUF_SIZE = 200;


constexpr int MAX_USER = 10000;
constexpr int MAX_NPC = 200000;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int VIEW_RANGE = 7;
constexpr int AGRO_ACTIVE_RANGE = 5;


// Packet ID

constexpr char CS_REGIST = 6;

constexpr char CS_LOGIN = 0;
constexpr char CS_LOGIN_STRESS = 10;
constexpr char CS_MOVE = 1;
constexpr char CS_CHAT = 2;
constexpr char CS_ATTACK = 3;			
constexpr char CS_TELEPORT = 4;			
constexpr char CS_LOGOUT = 5;			
constexpr char CS_CHOICE_CHARACTER = 9;


constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_LOGIN_FAIL = 3;
constexpr char SC_ADD_OBJECT = 4;
constexpr char SC_REMOVE_OBJECT = 5;
constexpr char SC_MOVE_OBJECT = 6;
constexpr char SC_CHAT = 7;
constexpr char SC_STAT_CHANGE = 8;
constexpr char SC_CHOICE_CHARACTER = 9;
constexpr char SC_ATTACK_SUCCESS = 10;



//error code
constexpr int NONE = 2001;
constexpr int LOGIN_OK = 2002;
constexpr int LOGIN_FAIL = 2003;	//ID 중복일때
constexpr int LOGIN_FAIL_MAXUSER = 2004;
constexpr int NOT_EXIST_IN_DB = 2005;
constexpr int NOT_CHOICE_CHARACTER = 2006;

//MonsterType
constexpr int PEACE_FIXED = 11;		//빨간색
constexpr int PEACE_ROAMING = 12;	//노란색
constexpr int AGRO_FIXED = 13;		//초록색
constexpr int AGRO_ROAMING = 14;		//파란색



enum DIRECTION { LEFT = 2, RIGHT = 3, UP = 0, DOWN = 1 ,ALL = 5};


#pragma pack (push, 1)

//내가 추가한것들
struct PACKET_HEADER {
	unsigned short size;
	char	type;
};

//클라->서버
struct CS_REGIST_PACKET :public PACKET_HEADER
{
	char	name[NAME_SIZE];
	char	password[NAME_SIZE];

};

struct CS_CHOICECHAR_PACKET :public PACKET_HEADER
{
	int visual;
};

struct CS_ATTACK_PACKET : public PACKET_HEADER 
{
	char	dir;

};

//서버->클라이언트
struct SC_CHOICECHAR_PACKET :public PACKET_HEADER
{
};
struct SC_SUC_ATTACK_PACKET :public PACKET_HEADER
{
	int npcId;
	int damage;
};



//클라이언트 -> 서버
struct CS_LOGIN_PACKET {
	unsigned short size;
	char	type;
	char	name[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned short size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned	move_time;
};

struct CS_CHAT_PACKET {
	unsigned short size;			
	char	type;
	char	mess[CHAT_SIZE];
};

struct CS_TELEPORT_PACKET {			
	char	type;
};

struct CS_LOGOUT_PACKET {
	unsigned short size;
	char	type;
};


//서버 -> 클라이언트
struct SC_LOGIN_INFO_PACKET {
	unsigned short size;
	char	type;
	int		visual;				
	int		id;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
	short	x, y;
};

struct SC_ADD_OBJECT_PACKET {
	unsigned short size;
	char	type;
	int		id;
	int		visual;				
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned short size;
	char	type;
	int		id;
};

struct SC_MOVE_OBJECT_PACKET {
	unsigned short size;
	char	type;
	int		id;
	short	x, y;
	char	dir;	
	unsigned int move_time;
};

struct SC_CHAT_PACKET {
	unsigned short size;
	char	type;
	int		id;
	char	mess[CHAT_SIZE];
};

struct SC_LOGIN_FAIL_PACKET {
	unsigned short size;
	char	type;
};

struct SC_STAT_CHANGE_PACKET {
	unsigned short size;
	char	type;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;

};

#pragma pack (pop)