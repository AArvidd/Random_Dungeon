#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "locale.h"
#include "wchar.h"
#include "termios.h"
#include "unistd.h"
#include "math.h"

#define width 100
#define height 100

#define world_width 7
#define world_height 7

#define queue_lenth 200

#define max_bolts 50

#define bolt_flag (1 << 31)
#define item_flag (1 << 30)
#define consumables_flag (1 << 29)
#define all_flages (bolt_flag | item_flag | consumables_flag) 

#define max_items 50
#define inventory_size 20

#define search_cross 4
#define search_full 8

#define full_bright 0

typedef enum{
    tile_floor,
    tile_rock,
    tile_hardrock,
    tile_wall,
    tile_door,
    tile_exit,
}tile_type;

typedef enum{
    left,
    right,
    upp,
    down,
}direktions;

typedef enum{
    no_enemie,
    goblin,
    golem,
    bicorn,
    lion,
    wizard,

    enemies_types_amount,

}enemies_enum;

typedef enum{
    map_normal,
    map_world,
}mape_modes_e;

typedef struct{
    int curent;
    int next_empty;
    int queue_x[queue_lenth];
    int queue_y[queue_lenth];
}queue_t;

typedef struct{
    int exits_x[4];
    int exits_y[4];
    int viseted;
    int sead;
    int conection;
}world_tile_t;

typedef struct{
    char* name;
    int hp;
    int hp_regen;
    int attack; 
    int mp;
    int mp_regen;
    int defense;
    int slots;
    int x;
    int y;
    int aktive;
}items_t;

typedef struct{
    char name[32];
    int type;
}spels_t;

typedef struct{
    char* name;
    int hp;
    int mp;
    int x;
    int y;
}consumables_t;

typedef struct{
    items_t items[inventory_size];
    spels_t spels[10];
    consumables_t consumables[inventory_size];
    int items_amount;
    int spels_amount;
    int consumables_amount;
}inventory_t;

typedef struct{
    int damage;
    int defense;
    int hp;
    int mp;
    int max_hp;
    int max_mp;
    int hp_regen;
    int mp_regen;
    int dir;
    int x;
    int y;
    inventory_t inventory;
    int equipment[7]; 
}player_t;

typedef struct{
    int deapth;
    float start_slope;
    float end_slope;
}ROW;

typedef struct{
    int dir;
    int origin_x;
    int origin_y;
}quadrent_t;

typedef struct{
    int hp;
    int damage;
    int spead;
    int movement_x[8];
    int movement_y[8];
    int movment_option;
}enemies_type_t;

typedef struct{
    int x;
    int y;
    int hp;
    int to_x;
    int to_y;
    int path_x[256];
    int path_y[256];
    int steps;
    int seen;
    int type;
    int movment;
    int flags;
    int timer;
}enemies_t;

typedef struct{
    int x;
    int y;
    int dir_x;
    int dir_y;
    int damage;
}bolt_t;

int map1[width][height];    // terain
int map2[width][height];    // enteties / data fore map generation
int map3[width][height];    // visibility
int map4[width][height];    // enemy path finding / other data

int world_pos_x;
int world_pos_y;

int map_mode = 0;

world_tile_t world_map[world_width][world_height];

player_t player = {.damage = 2, .hp = 20, .inventory = {.spels[0] = {.name = "Fierball"}, .spels_amount = 1}};

enemies_t enemies[100];
int aktive_enemies;

enemies_type_t enemies_types[] = {
    { 0 },
    {.damage = 1, .hp = 10, .spead = 0, .movment_option = 4, .movement_x = {-1,  1,  0,  0}, .movement_y = {0,  0, -1,  1}},
    {.damage = 2, .hp = 20, .spead = 2, .movment_option = 4, .movement_x = {-1,  1,  0,  0}, .movement_y = {0,  0, -1,  1}},
    {.damage = 1, .hp =  8, .spead = 0, .movment_option = 8, .movement_x = {-1,  1,  1, -1,  2, -2,  2, -2}, .movement_y = {-2,  2, -2,  2, -1,  1,  1, -1}},
    {.damage = 2, .hp =  8, .spead = 0, .movment_option = 8, .movement_x = {-2,  2,  0,  0, -1,  1,  0,  0}, .movement_y = { 0,  0, -2,  2,  0,  0, -1,  1}},
    {.damage = 2, .hp =  5, .spead = 1, .movment_option = 4, .movement_x = {-1,  1,  0,  0}, .movement_y = {0,  0, -1,  1}},
};

bolt_t bolts[max_bolts];

items_t items[max_items];
consumables_t consumables[max_items];

char* item_names[] = {
    "Sword          ",
    "Shield         ", 
    "Greatsword     ", 
    "Warhammer      ", 
    "Plait mail     ", 
    "Chain mail     ", 
    "Mage robe      ", 
    "Boots          ", 
    "Helmet         ", 
    "Pointy hat     ", 
    "Healing ring   ", 
    "Magic ring     ", 
    "Ring of Defense", 
    "Ring of Offense"};
char* consumabals_names[] = {"Stake", "Potion", "Soup", "Scrolls"};

int spatern_x[] = {-1,  1,  0,  0, -1,  1, -1,  1};
int spatern_y[] = { 0,  0, -1,  1, -1, -1,  1,  1};


int wallchar[] = {0x256c, 0x2550, 0x2550, 0x2550, 0x2551, 0x255d, 0x255a, 0x2569, 0x2551, 0x2557, 0x2554, 0x2566, 0x2551, 0x2563, 0x2560, 0x256c};



queue_t queue = {
    .curent = 0,
    .next_empty = 0
};

int conformation(char string[]){
    while(1){    
        printf("\x1b[1;1H\x1b[2J");
        printf("Do you want to %s? (y/n): ", string);
        char c;
        while((c = getchar()) == '\n');
        switch(c){
            case 'y':
            case 'Y':{
                printf("\x1b[1;1H\x1b[2J");
                return 1;
            }break;

            case 'n':
            case 'N':{
                printf("\x1b[1;1H\x1b[2J");
                return 0;
            }break;
        }
    }
}

int is_solid(int x, int y){
    if( map1[x][y] == tile_rock     ||
        map1[x][y] == tile_hardrock ||
        map1[x][y] == tile_wall     ||
        x < 0 || x > width - 1      ||
        y < 0 || y > height - 1
    ){
        return 1;
    }
    return 0;
}

//world map generation
void hunt_and_kill(int current_x, int current_y){
    while(1){
        world_map[current_x][current_y].viseted = 1;
        int dir = rand() % 4;

        int tests = 0;
        while(1){
            if( world_map[current_x + spatern_x[dir]][current_y + spatern_y[dir]].viseted ||
                current_x + spatern_x[dir] < 0 || current_x + spatern_x[dir] > world_width - 1 ||
                current_y + spatern_y[dir] < 0 || current_y + spatern_y[dir] > world_height - 1
            ){
                tests++;
                if(tests == 4){
                    return;
                }
                dir++;
                if(dir == 4){
                    dir = 0;
                }
                continue;
            }
            break;
        }

        world_map[current_x][current_y].conection |= 1 << dir;

        current_x += spatern_x[dir];
        current_y += spatern_y[dir];

        switch(dir){
            case 0:
                world_map[current_x][current_y].conection |= 1 << 1;
                break;
            case 1:
                world_map[current_x][current_y].conection |= 1 << 0;
                break;
            case 2:
                world_map[current_x][current_y].conection |= 1 << 3;
                break;
            case 3:
                world_map[current_x][current_y].conection |= 1 << 2;
                break;
        }
    }
}

void world_map_gen(){
    srand(time(NULL));
    for(int y = 0; y < world_height; y++){
        for(int x = 0; x < world_width; x++){
            for(int i = 0; i < 4; i++){
                if(!world_map[x + spatern_x[i]][y + spatern_y[i]].viseted && !(
                    x + spatern_x[i] < 0 || x + spatern_x[i] > world_width - 1 ||
                    y + spatern_y[i] < 0 || y + spatern_y[i] > world_height - 1
                )){
                    hunt_and_kill(x, y);
                    x = 0;
                    y = 0;
                    break;
                }
            }
        }
    }
    world_pos_x = rand() % world_width;
    world_pos_y = rand() % world_height;
}

//map generation
int flood_search(int x, int y, int marker){
    int area = 1;
    queue.queue_x[0] = x;
    queue.queue_y[0] = y;
    queue.next_empty++;
    map2[x][y] = 1;
    while(queue.curent != queue.next_empty){
        for(int i = 0; i < search_cross; i++){
            if((
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == tile_floor || 
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == tile_door) && 
                map2[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] != marker
            ){
                map2[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] = marker;
                area++;
                queue.queue_x[queue.next_empty] = queue.queue_x[queue.curent] + spatern_x[i];
                queue.queue_y[queue.next_empty] = queue.queue_y[queue.curent] + spatern_y[i];
                queue.next_empty++;
                if(queue.next_empty == queue_lenth){
                    queue.next_empty = 0;
                }

            }
        }
        queue.queue_x[queue.curent] = 0;
        queue.queue_y[queue.curent] = 0;
        queue.curent++;
        if(queue.curent == queue_lenth){
            queue.curent = 0;
        }
    }
    queue.curent = 0;
    queue.next_empty = 0;
    return area;
}

void flood_remove(int x, int y){
    queue.queue_x[0] = x;
    queue.queue_y[0] = y;
    queue.next_empty++;
    map1[x][y] = 1;
    while(queue.curent != queue.next_empty){
        for(int i = 0; i < search_cross; i++){
            if(map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] == tile_floor){
                map1[queue.queue_x[queue.curent] + spatern_x[i]][queue.queue_y[queue.curent] + spatern_y[i]] = 1;
                queue.queue_x[queue.next_empty] = queue.queue_x[queue.curent] + spatern_x[i];
                queue.queue_y[queue.next_empty] = queue.queue_y[queue.curent] + spatern_y[i];
                queue.next_empty++;
                if(queue.next_empty == queue_lenth){
                    queue.next_empty = 0;
                }

            }
        }
        queue.queue_x[queue.curent] = 0;
        queue.queue_y[queue.curent] = 0;
        queue.curent++;
        if(queue.curent == queue_lenth){
            queue.curent = 0;
        }
    }
    queue.curent = 0;
    queue.next_empty = 0;
}

void add_hardrock(){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){

            if(map1[x][y] == 0){
                continue;
            }

            int deap = 1;
            for(int x_off = -3; x_off < 4; x_off++){
                for(int y_off = -3; y_off < 4; y_off++){

                    if(x + x_off < 0 || x + x_off >= width || y + y_off < 0 || y + y_off >= height){
                        continue;
                    }else if(map1[x + x_off][y + y_off] == tile_floor){
                        deap = 0;
                        goto next;
                    }

                }
            }

            next:;

            if(deap){
                map1[x][y] = tile_hardrock;
            }

        }
    }
}

void coredor_gen(int from_x, int from_y, int to_x, int to_y){

    int moledir_x = to_x - from_x;
    int moledir_y = to_y - from_y;

    if(moledir_x < 0){
        moledir_x = -1;
    }else if(moledir_x > 0){
        moledir_x = 1;
    }

    if(moledir_y < 0){
        moledir_y = -1;
    }else if(moledir_y > 0){
        moledir_y = 1;
    }

    int olde_x = from_x;
    int olde_y = from_y;
    while(from_x != to_x || from_y != to_y){
        int oldest_x = olde_x;
        int oldest_y = olde_y;
        olde_x = from_x;
        olde_y = from_y;
        if(from_x != to_x){
            from_x += moledir_x;
        } else{
            from_y += moledir_y;
        }

        if(map1[from_x][from_y] == tile_wall || map1[from_x][from_y] == tile_door){
            map1[from_x][from_y] = tile_door;
        }else{
            map1[from_x][from_y] = tile_floor;
        }

        for(int i = 0; i < search_full; i++){
            if(map1[oldest_x + spatern_x[i]][oldest_y + spatern_y[i]] == tile_rock){
                map1[oldest_x + spatern_x[i]][oldest_y + spatern_y[i]] = tile_wall;
            }
        }
    }
}

void tunel_gen(int from_x, int from_y, int direction){
    while(map1[from_x][from_y] != tile_floor){
        map1[from_x][from_y] = tile_floor;
        from_x += spatern_x[direction];
        from_y += spatern_y[direction];
    }

}

void cave_gen(int acces){
    //initial sead
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int temp = (rand() % 100) + 1;
            if(temp >= 70){
                map1[x][y] = tile_rock;
            }else{
                map1[x][y] = tile_floor;
            }
        }
    }

    //seluar atatmeta
    for(int round = 0; round < 5; round++){
        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){

                int neighbors = 0;
                for(int i = 0; i < search_full; i++){
                    if(x + spatern_x[i] < 0 || x + spatern_x[i] >= width || y + spatern_y[i] < 0 || y + spatern_y[i] >= height){
                        neighbors++;
                    }else if(map1[x + spatern_x[i]][y + spatern_y[i]]){
                        neighbors++;
                    }
                }
                if(neighbors >= 4){
                    map2[x][y] = tile_rock;
                }else if(neighbors == 3){
                    map2[x][y] = map1[x][y];
                }else{
                    map2[x][y] = tile_floor;
                }

            }
        }

        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
                map1[x][y] = map2 [x][y];
            }
        }
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map2[x][y] = 0;
        }
    }

    //remove too smale / conect disconected arias
    int search = 0;

    int remove_x[20];
    int remove_y[20];
    int remove = 0;

    int keep_x[20];
    int keep_y[20];
    int keep = 0;

    int largest = 0;
    int largest_id = 0;
    int largest_search = 0; 

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(map1[x][y] == 0 && map2[x][y] == 0){
                search++;
                int area = flood_search(x, y, search);
                if(area < 11){
                    remove_x[remove] = x;
                    remove_y[remove] = y;
                    remove++;
                }else{
                    keep_x[keep] = x;
                    keep_y[keep] = y;

                    if(area > largest){
                        largest = area;
                        largest_id = keep;
                        largest_search = search;
                    }

                    keep++;
                }
            }
        }
    }

    for(int i = 0; i < remove; i++){
        flood_remove(remove_x[i], remove_y[i]);
    }

    for(int i = 0; i < keep; i++){
        if(i == largest_id){
            continue;
        }
        int x = keep_x[i];
        int y = keep_y[i];
        int id = map2[x][y];

        for(int j = 0; j < 20; j++){
            int dir = rand() % search_cross;
            if(map2[x + spatern_x[dir]][y + spatern_y[dir]] != id){
                j--;
                continue;
            }
            x += spatern_x[dir];
            y += spatern_y[dir];
        }

        int dir = rand() % search_cross;
        int temp_x = x;
        int temp_y = y;
        while(map2[temp_x][temp_y] != largest_search){
            temp_x += spatern_x[dir];
            temp_y += spatern_y[dir];
            if(temp_x < 0 || temp_x > width - 1 || temp_y < 0 || temp_y > height - 1){
                temp_x = x;
                temp_y = y;
                dir++;
                if(dir == search_cross){
                    dir = 0;
                }
            }
        }

        while(map1[x][y] != tile_rock){
            x += spatern_x[dir];
            y += spatern_y[dir];
        }
        tunel_gen(x, y, dir);
        
    }

    //exit tunels
    if(acces & 1 << 0){
        int exit = rand() % (height - 2) + 1;
        tunel_gen(0, exit, right);
        map1[0][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[2] = 0;
        world_map[world_pos_x][world_pos_y].exits_y[2] = exit;
    }
    if(acces & 1 << 1){
        int exit = rand() % (height  - 2) + 1;
        tunel_gen(width - 1, exit, left);
        map1[width - 1][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[3] = width - 1;
        world_map[world_pos_x][world_pos_y].exits_y[3] = exit;
    }
    if(acces & 1 << 2){
        int exit = rand() % (width - 2) + 1;
        tunel_gen(exit, 0, down);
        map1[exit][0] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[0] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[0] = 0;
    }
    if(acces & 1 << 3){
        int exit = rand() % (width - 2) + 1;
        tunel_gen(exit, height - 1, upp);
        map1[exit][height - 1] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[1] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[1] = height - 1;
    }

}

void dungeon_gen(int acces){
    //room generation

    for(int x = 0; x <width + 1; x++){
        for(int y = 0; y <height + 1 ; y++){
            map1[x][y] = 1;
        }
    }

    int max_rooms = 10;


    int rooms_x[max_rooms];
    int rooms_y[max_rooms];
    int rooms = 0;

    int attempts = 0;
    while(rooms < max_rooms){
        int room_width = (rand() % 11) + 5;
        int room_height = (rand() % 11) + 5;
        int room_x = (rand() % width - 1) + 1;
        int room_y = (rand() % height - 1) + 1;

        int clear = 1;

        for(int x = -1; x < room_width + 1; x++){
            for(int y = -1; y < room_height + 1; y++){
                if(map1[room_x + x][room_y + y] == 0 || room_x + x < 0 || room_x + x >= width || room_y + y < 0 || room_y + y >= height){
                    clear = 0;
                    attempts++;
                    goto exit;
                }
            }
        }
        exit:;

        if(attempts == 10){
            break;
        }
        if(!clear){
            continue;
        }

        rooms_x[rooms] = room_x + (int)(room_width / 2);
        rooms_y[rooms] = room_y + (int)(room_height / 2);

        for(int x = -1; x <= room_width; x++){
            for(int y = -1; y <= room_height; y++){
                map1[room_x + x][room_y + y] = tile_floor;
                if(x == -1 || x == room_width || y == -1 || y == room_height){
                    map1[room_x + x][room_y + y] = tile_wall;
                }
            }
        }
        rooms++;
        attempts = 0;
    }

    //coridor generation

    for(int i = 0; i < rooms - 1; i++){
        int room1 = rand() % rooms;
        int room2;
        while((room2 = rand() % rooms) == room1);

        coredor_gen(rooms_x[room1], rooms_y[room1], rooms_x[room2], rooms_y[room2]);
    }

    while(1){
        int areas = 0;
        for(int i = 0; i < rooms; i++){
            if(map2[rooms_x[i]][rooms_y[i]] == 0){
                areas++;
                flood_search(rooms_x[i], rooms_y[i], areas);
            }
        }

        if(areas == 1){
            break;
        }

        int room1 = rand() % rooms;
        int room2;
        while((room2 = rand() % rooms) == room1);

        if(map2[rooms_x[room1]][rooms_y[room1]] != map2[rooms_x[room2]][rooms_y[room2]]){
            coredor_gen(rooms_x[room1], rooms_y[room1], rooms_x[room2], rooms_y[room2]);
        }

        for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
                map2[x][y] = 0;
            }
        }
    }

    //exit coredor gen
    if(acces & 1 << 0){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_x[i] < rooms_x[furthest]){
                furthest = i;
            }
        }
        coredor_gen(0, exit, rooms_x[furthest], rooms_y[furthest]);
        map1[0][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[2] = 0;
        world_map[world_pos_x][world_pos_y].exits_y[2] = exit;
    }
    if(acces & 1 << 1){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_x[i] > rooms_x[furthest]){
                furthest = i;
            }
        }
        coredor_gen(width - 1, exit, rooms_x[furthest], rooms_y[furthest]);
        map1[width - 1][exit] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[3] = width - 1;
        world_map[world_pos_x][world_pos_y].exits_y[3] = exit;
    }
    if(acces & 1 << 2){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_y[i] < rooms_y[furthest]){
                furthest = i;
            }
        }
        coredor_gen(rooms_x[furthest], rooms_y[furthest], exit, 0);
        map1[exit][0] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[0] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[0] = 0;

    }
    if(acces & 1 << 3){
        int exit = rand() % width;
        int furthest = 0;
        for(int i = 1; i < rooms; i++){
            if(rooms_y[i] > rooms_y[furthest]){
                furthest = i;
            }
        }
        coredor_gen(rooms_x[furthest], rooms_y[furthest], exit, height -1);
        map1[exit][height - 1] = tile_exit;
        world_map[world_pos_x][world_pos_y].exits_x[1] = exit;
        world_map[world_pos_x][world_pos_y].exits_y[1] = height - 1;
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(map1[x][y] == tile_door){
                if( (map1[x + spatern_x[0]][y + spatern_y[0]] == tile_wall && map1[x + spatern_x[1]][y + spatern_y[1]] == tile_wall) || 
                    (map1[x + spatern_x[2]][y + spatern_y[2]] == tile_wall && map1[x + spatern_x[3]][y + spatern_y[3]] == tile_wall)
                ){
                    map1[x][y] = tile_door;
                }else{
                    map1[x][y] = tile_floor;
                }
            }
        }
    }

}

void enemies_gen(){
    aktive_enemies = 0;
    for(int i = 0; i < 20; i++){
        int x = rand() % width;
        int y = rand() % height;
        while(map1[x][y] != 0 || map2[x][y] != 0){
            x = rand() % width;
            y = rand() % height;    
        }
        int type = rand() % (enemies_types_amount - 1) + 1;
        // int type = bicorn;
        int movment = rand() % (enemies_types[type].spead + 1);
        enemies[i] = (enemies_t){.hp = enemies_types[type].hp, .x = x, .y = y, .steps = -1, .type = type};
        map2[x][y] = type;
        aktive_enemies++;
    }
}

void plase_player(){
    player.x = rand() % width;
    player.y = rand() % height;
    while(map1[player.x][player.y] != 0){
        player.x = rand() % width;
        player.y = rand() % height;    
    }
}

void map_gen(){
    if(world_map[world_pos_x][world_pos_y].sead == 0){
        world_map[world_pos_x][world_pos_y].sead = time(NULL);
    }
    srand(world_map[world_pos_x][world_pos_y].sead);
    if(rand() % 2){
        dungeon_gen(world_map[world_pos_x][world_pos_y].conection);
    }else{
        cave_gen(world_map[world_pos_x][world_pos_y].conection);
    }
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map2[x][y] = 0;
        }
    }
    enemies_gen();
    add_hardrock();
}

// enemy movment
void kill_enemie(int i){
    map2[enemies[i].x][enemies[i].y] = 0;
    if(rand() % 3 && 0){
        return;
    }
    int value = rand() % 15 + 2;
    if(rand() % 2 && 0){
        int temp[2];
        int first = rand() % 2;
        temp[first] = rand() % value;
        value -= temp[first];
        temp[!first] = rand() % value;

        consumables_t consumables_temp = {.hp = temp[0], .mp = temp[1], .name = consumabals_names[rand() % 4], .x = enemies[i].x, .y = enemies[i].y};

        for(int j = 0; j < max_items; j++){
            if(consumables[j].hp == 0 && consumables[i].mp == 0){
                consumables[j] = consumables_temp;
                map2[enemies[i].x][enemies[i].y] = j | consumables_flag;
                return;
            }
        }

    }else{
        int slots = 0;
        int hp = 0;
        int hp_regen = 0;
        int attack = 0; 
        int mp = 0;
        int mp_regen = 0;
        int defense = 0;
        char* name;

        switch(rand() % 7){
            case 0:{ // main hand
                slots = 1;
                attack = rand() % 15 + 1;
                if(rand() % 2){
                    defense = rand() % 5;
                }
                name = item_names[0];
            }break;
            case 1:{ // off hand
                slots = 2;
                defense = rand() % 15 + 1;
                if(rand() % 2){
                    attack = rand() % 5;
                }
                name = item_names[1];
            }break;
            case 2:{ // doule hand
                slots = 3;
                attack = rand() % 15 + 10;
                name = item_names[rand() % 2 + 2];
            }break;
            case 3:{ // armor
                slots = 4;
                if(rand() % 2){
                    hp = rand() % 15 + 1;
                    defense = rand() % 10 + 1;
                    name = item_names[rand() % 2 +4];
                }else{
                    hp = rand() % 5 + 1;
                    mp = rand() % 15 + 10;
                    mp_regen = rand() % 5;
                    name = item_names[6];
                }
            }break;
            case 4:{ // bots
                slots = 8;
                if(rand() % 2){
                    hp = rand() % 10 + 1;
                    defense = rand() % 5 + 1;
                }else{
                    hp = rand() % 3 + 1;
                    mp = rand() % 10 + 10;
                    mp_regen = rand() % 3;
                }
                name = item_names[7];
            }break;
            case 5:{ // helmet
                slots = 16;
                if(rand() % 2){
                    hp = rand() % 10 + 1;
                    defense = rand() % 5 + 1;
                    name = item_names[8];
                }else{
                    hp = rand() % 3 + 1;
                    mp = rand() % 10 + 10;
                    mp_regen = rand() % 3;
                    name = item_names[9];
                }
            }break;
            case 6:{ // rings
                slots = 32;
                switch(rand() % 4){
                    case 0:{ // hp
                        hp = rand() % 10 + 1;
                        hp_regen = rand() % 5;
                        name = item_names[10];
                    }break;
                    case 1:{ // mp
                        mp = rand() % 10 + 1;
                        mp_regen = rand() & 5;
                        name = item_names[11];
                    }break;
                    case 2:{ // defense
                        defense = rand() % 15 + 1;
                        name = item_names[12];
                    }break;
                    case 3:{ // offens
                        attack = rand() % 10 + 1;
                        name = item_names[13];
                    }break;
                }
            }break;
        }

        items_t item_temp = {.attack = attack, .defense = defense, .hp = hp, .hp_regen = hp_regen, .mp = mp, .mp_regen = mp_regen, .name = name, .slots = slots, .x = enemies[i].x, .y = enemies[i].y, .aktive = 1};

        for(int j = 0; j < max_items; j++){
            if(items[j].aktive == 0){
                items[j] = item_temp;
                map2[enemies[i].x][enemies[i].y] = j | item_flag;
                return;
            }
        }
    }
}

float distens(int x1, int y1, int x2, int y2){
    int x = x1 - x2;
    int y = y1 - y2;

    x *= x;
    y *= y;

    return sqrt(x + y);
}

void bolt_spawn(int x, int y, int dir_x, int dir_y, int damage){
    for(int i = 0; i < max_bolts; i++){
        if(bolts[i].damage == 0){
            bolts[i] = (bolt_t){.x = x, .y = y, .dir_x = dir_x, .dir_y = dir_y, .damage = damage};
            return;
        }
    }
}

void bolt_uppdate(){
    for(int i = 0; i < max_bolts; i++){
        if(bolts[i].damage == 0){
            continue;
        }

        int dx = abs(bolts[i].dir_x);
        int dy = abs(bolts[i].dir_y);
        int next_x = bolts[i].x + bolts[i].dir_x;
        int next_y = bolts[i].y + bolts[i].dir_y;
        int x_inc = (next_x > bolts[i].x) ? 1 : -1;
        int y_inc = (next_y > bolts[i].y) ? 1 : -1;

        int error = dx - dy;
        int n = dx + dy;

        dx *= 2;
        dy *= 2;

        if(n == 0){
            n = 1;
            bolts[i].damage = 0;
        }

        for(; n > 0; --n){
            if(error > 0){
                bolts[i].x += x_inc;
                error -= dy;
            }else if(error < 0){
                bolts[i].y += y_inc;
                error += dx;
            }else{
                bolts[i].x += x_inc;
                bolts[i].y += y_inc;
                error -= dy;
                error += dx;
                --n;
            }

            if(is_solid(bolts[i].x, bolts[i].y) || map1[bolts[i].x][bolts[i].y] == tile_door){
                bolts[i].damage = 0;
            }else if(map2[bolts[i].x][bolts[i].y]){
                for(int j = 0; j < aktive_enemies; j++){
                    if(enemies[j].x == bolts[i].x && enemies[j].y == bolts[i].y){
                        enemies[j].hp -= bolts[i].damage;
                        bolts[i].damage = 0;
                        if(enemies[j].hp <= 0){
                            kill_enemie(i);
                        }
                        break;
                    }
                }
            }else if(bolts[i].x == player.x && bolts[i].y == player.y){
                player.hp -= bolts[i].damage;
                bolts[i].damage = 0;
            }
            if(bolts[i].damage == 0){
                for(int x = -4; x < 5; x++){
                    for(int y = -4; y < 5; y++){
                        if(distens(bolts[i].x, bolts[i].y, bolts[i].x + x, bolts[i].y + y) < (random() % 4) + 1){
                            map4[bolts[i].x + x][bolts[i].y + y] = 1;
                        }
                    }
                }
                break;
            }

        }
        map2[bolts[i].x][bolts[i].y] = 1 | bolt_flag;

    }
}

void explotion(){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            if(map4[x][y]){
                if(map1[x][y] != tile_exit && map1[x][y] != tile_hardrock){
                    map1[x][y] = tile_floor;
                }
                if(map2[x][y] && !(map2[x][y] & all_flages)){

                    for(int i = 0; i < aktive_enemies; i++){
                        if(enemies[i].x == x && enemies[i].y == y){
                            enemies[i].hp--;
                            break;
                        }
                    }

                }else if(player.x == x && player.y == y){
                    player.hp--;
                }
            }
        }
    }
}

void enemies_pathfinding(int i){
    int no_path = 0;
    for(int j = 0; j < queue_lenth; j++){
        queue.queue_x[j] = -1;
        queue.queue_y[j] = -1;
    }
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map4[x][y] = -1;
        }
    }

    queue.queue_x[0] = enemies[i].x;
    queue.queue_y[0] = enemies[i].y;
    map4[enemies[i].x][enemies[i].y] = 0;
    // queue.next_empty++;

    while(1){

        int next = 0;
        for(int j = 1; j < queue_lenth; j++){
            if(queue.queue_x[j] == -1 || queue.queue_y[j] == -1){
                continue;
            }
            if(map4[queue.queue_x[next]][queue.queue_y[next]] > map4[queue.queue_x[j]][queue.queue_y[j]] || queue.queue_x[next] == -1){
                next = j;
            }
        }
        if(next == 0 && queue.queue_x[next] == -1 && queue.queue_y[next] == -1){
            no_path = 1;
            break;
        }

        if(is_solid(queue.queue_x[next], queue.queue_y[next]) || map2[queue.queue_x[next]][queue.queue_y[next]] != 0){
            queue.queue_x[next] = -1;
            queue.queue_y[next] = -1;
            continue;
        }

        for(int j = 0; j < enemies_types[enemies[i].type].movment_option; j++){

            if(enemies[i].to_x == queue.queue_x[next] + enemies_types[enemies[i].type].movement_x[j] && enemies[i].to_y == queue.queue_y[next] + enemies_types[enemies[i].type].movement_y[j]){
                map4[enemies[i].to_x][enemies[i].to_y] = j ^ 1;
                goto exit;
            }

            // int g = map4[queue.queue_x[next]][queue.queue_y[next]] + 1;
            int g = 0;
            float h = distens(queue.queue_x[next] + enemies_types[enemies[i].type].movement_x[j], queue.queue_y[next] + enemies_types[enemies[i].type].movement_y[j], enemies[i].to_x, enemies[i].to_y);
            int f = (g + (int)ceil(h)) << 3;

            if(/* f >= (map4[queue.queue_x[next] + enemies_types[enemies[i].type].movement_x[j]][queue.queue_y[next] + enemies_types[enemies[i].type].movement_y[j]] & ~3) &&  */
                map4[queue.queue_x[next] + enemies_types[enemies[i].type].movement_x[j]][queue.queue_y[next] + enemies_types[enemies[i].type].movement_y[j]] != -1
            ){
                continue;
            }

            f |= j ^ 1;
            map4[queue.queue_x[next] + enemies_types[enemies[i].type].movement_x[j]][queue.queue_y[next] + enemies_types[enemies[i].type].movement_y[j]] = f;

            for(int k = 0; k < queue_lenth; k++){
                if(queue.queue_x[k] == -1 && queue.queue_y[k] == -1){
                    queue.queue_x[k] = queue.queue_x[next] + enemies_types[enemies[i].type].movement_x[j];
                    queue.queue_y[k] = queue.queue_y[next] + enemies_types[enemies[i].type].movement_y[j];
                    break;
                }
            }

        }
        queue.queue_x[next] = -1;
        queue.queue_y[next] = -1;
    }
    exit:;

    if(!no_path){
        int follower_x = enemies[i].to_x;
        int follower_y = enemies[i].to_y;

        enemies[i].steps = 0;


        while(follower_x != enemies[i].x || follower_y != enemies[i].y){
            enemies[i].path_x[enemies[i].steps] = follower_x;
            enemies[i].path_y[enemies[i].steps] = follower_y;

            int dir = map4[follower_x][follower_y] & 7;
            follower_x += enemies_types[enemies[i].type].movement_x[dir];
            follower_y += enemies_types[enemies[i].type].movement_y[dir];
            enemies[i].steps++;
        }
        enemies[i].steps--;
    }

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map4[x][y] = 0;
        }
    }

}

void enemies_uppdete(){

    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map2[x][y] = 0;
        }
    }

    for(int i = 0; i < aktive_enemies; i++){
        if(enemies[i].hp > 0){
            map2[enemies[i].x][enemies[i].y] = enemies[i].type;
        }
    }

    for(int i = 0; i < aktive_enemies; i++){
        if(enemies[i].hp <= 0){
            continue;
        }
        map2[enemies[i].x][enemies[i].y] = 0;
        if(map3[enemies[i].x][enemies[i].y] == 1 || enemies[i].seen){
            enemies[i].to_x = player.x;
            enemies[i].to_y = player.y;
            enemies_pathfinding(i);
        }

        enemies[i].seen = 0;
        if(map3[enemies[i].x][enemies[i].y] == 1){
            enemies[i].seen = 1;
        }
        if(enemies[i].steps != -1){
            if(enemies[i].movment == 0){
                enemies[i].movment = enemies_types[enemies[i].type].spead;

                if(enemies[i].type == wizard){
                    if(map3[enemies[i].x][enemies[i].y] == 1 && 100/(distens(enemies[i].x, enemies[i].y, player.x, player.y) / 2) > random() % 40 + 1){
                        float disten = distens(enemies[i].x, enemies[i].y, player.x, player.y);
                        float dir_x = (player.x - enemies[i].x) / disten;
                        float dir_y = (player.y - enemies[i].y) / disten;
                        dir_x *= 2;
                        dir_y *= 2;
                        dir_x = round(dir_x);
                        dir_y = round(dir_y);
                        bolt_spawn(enemies[i].x , enemies[i].y, dir_x, dir_y, enemies_types[enemies[i].type].damage);

                    }else if(map2[enemies[i].path_x[enemies[i].steps]][enemies[i].path_y[enemies[i].steps]] && !(map2[enemies[i].path_x[enemies[i].steps]][enemies[i].path_y[enemies[i].steps]] & all_flages)){
                        enemies_pathfinding(i);
                    }else{
                        enemies[i].x = enemies[i].path_x[enemies[i].steps];
                        enemies[i].y = enemies[i].path_y[enemies[i].steps];
                        enemies[i].steps--;
                    }
                }else{
                    if(enemies[i].path_x[enemies[i].steps] == player.x && enemies[i].path_y[enemies[i].steps] == player.y){
                        int damage = enemies_types[enemies[i].type].damage - player.defense;
                        if(damage < 1){
                            damage = 1;
                        }
                        player.hp -= damage;
                    }else if(map2[enemies[i].path_x[enemies[i].steps]][enemies[i].path_y[enemies[i].steps]] && !(map2[enemies[i].path_x[enemies[i].steps]][enemies[i].path_y[enemies[i].steps]] & all_flages)){
                        enemies_pathfinding(i);
                    }else{
                        enemies[i].x = enemies[i].path_x[enemies[i].steps];
                        enemies[i].y = enemies[i].path_y[enemies[i].steps];
                        enemies[i].steps--;
                    }
                }
            }else{
                enemies[i].movment--;
            }
        }
        if(map2[enemies[i].x][enemies[i].y]){
            map4[enemies[i].x][enemies[i].y] = 2;
        }
        map2[enemies[i].x][enemies[i].y] = enemies[i].type;

    }
}

// map render
void plase_items(){
    for(int i = 0; i < max_items; i++){
        if(items[i].aktive){
            map2[items[i].x][items[i].y] = i | item_flag;
        }
        if(consumables[i].hp || consumables[i].mp){
            map2[consumables[i].x][consumables[i].y] = i | consumables_flag;
        }
    }
}

void draw_wall(int x, int y){
    int type = 0;

    for(int i = 0; i < search_cross; i++){
        if(map1[x + spatern_x[i]][y + spatern_y[i]] == tile_wall || map1[x + spatern_x[i]][y + spatern_y[i]] == tile_door){
            type |= 1 << i;
        }
    }

    if(type & 1){
        wprintf(L"%lc", wallchar[1]);
    }else{
        printf(" ");
    }

    wprintf(L"%lc", wallchar[type]);

    if(type & 2){
        wprintf(L"%lc", wallchar[1]);
    }else{
        printf(" ");
    }
    
}

void draw_tile(int x, int y){
    switch(map1[x][y]){
        case tile_floor:
            if(full_bright){
                printf("   ");
            }else{
                printf(" . ");
            }
            break;
        case tile_rock:
            printf(" # ");
            break;
        case tile_hardrock:
            printf(" & ");
            break;
        case tile_wall:
            draw_wall(x, y);
            break;
        case tile_door:
            printf(" D ");
            break;
        case tile_exit:
            printf(" E ");
            break;
        default:
            printf("???");
    }
}

void draw_enemie(int i){
    printf(" ");
    switch(i){
        case goblin:
            printf("\e[32mG");
            break;
        case golem:
            printf("\e[90m&");
            break;
        case bicorn:
            printf("\e[35mM");
            break;
        case lion:
            printf("\e[33mL");
            break;
        case wizard:
            printf("\e[35mW");
            break;

        default:
            printf("\e[31m?");
            break;
    }
    printf(" \e[0m");
}

void draw_bolt(int x, int y){

    printf(" \e[33mo\e[0m ");

    // int i = 0;
    // for(; i < max_bolts; i++){
    //     if(bolts[i].x == x && bolts[i].y == y){
    //         break;
    //     }
    // }

    // float angle = atan2(bolts[i].dir_x, bolts[i].dir_y);

   

}

void draw_map(){
    if(map_mode == map_normal){
        printf("       ");
        for(int x = 0; x < width; x++){
            printf("%2d ", x);
        }
        printf("\n");

        for(int y = 0; y < height; y++){
            printf("y = %2d:", y);
            for(int x = 0; x < width; x++){
                if(map4[x][y] == 1){
                    printf("\x1b[43m");
                }
                if(map4[x][y] == 2){
                    printf("\x1b[41m");
                }
                if(x == player.x && y == player.y){
                    printf("\x1b[32m @ \x1b[0m");
                    continue;
                }
                if(map3[x][y] == 1 || full_bright){
                    if(map2[x][y] & bolt_flag){
                        draw_bolt(x, y);
                    }else if(map2[x][y] & item_flag){
                        printf("\x1b[90m § \x1b[0m");
                    }else if(map2[x][y] & consumables_flag){
                        printf("\x1b[90m %c \x1b[0m", '%');
                    }else if(map2[x][y] != 0){
                        draw_enemie(map2[x][y]);
                    }else{
                        draw_tile(x, y);
                    }
                }else{
                    printf("\x1b[0m   ");
                }
                printf("\x1b[0m");
            }
            printf("\n");
        }
        printf("\nMove: W, A, S, D");
        printf("   HP = %2d / %2d", player.hp, player.max_hp);
        printf("   MP = %2d / %2d", player.mp, player.max_mp);
        printf("   Damage = %2d   Defense = %2d", player.damage, player.defense);
        printf("\n");

    }else if(map_mode == map_world){
        printf("world pos: %d, %d\n", world_pos_x, world_pos_y);
        for(int y = 0; y < world_height; y++){
            printf("y: %2d ", y);
            for(int x = 0; x < world_width; x++){

                if(x == world_pos_x && y == world_pos_y){
                    printf("\x1b[31m");
                }

                wprintf(L"%lc", wallchar[world_map[x][y].conection]);
                if(world_map[x][y].conection & 1 << 1 && world_map[x + 1][y].conection & 1){
                    wprintf(L"%lc", wallchar[1]);
                }else{
                    printf(" ");
                }
                printf("\x1b[0m");

            }
            printf("\n");
        }
    }

}

void clear_information(){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            map3[x][y] = 0;
            map4[x][y] = 0;
        }
    }
}

// player
void draw_inventory(int type, int index){
    printf("\x1b[1;1H\x1b[2J");
    printf("Items:\n");
    for(int i = 0; i < player.inventory.items_amount; i++){
        if(type == 0 && index == i){
            printf("\x1b[31m");
        }
        printf("%2d: %s (", i + 1, player.inventory.items[i].name);

        int later = 0;

        items_t item_temp = player.inventory.items[i]; 

        if(item_temp.attack){
            printf("Attack = %d", item_temp.attack);
            later = 1;
        }
        if(item_temp.defense){
            if(later){
                printf(", ");
            }
            printf("Defense = %d", item_temp.defense);
            later = 1;
        }
        if(item_temp.hp){
            if(later){
                printf(", ");
            }
            printf("hp = %d", item_temp.hp);
            later = 1;
        }
        if(item_temp.hp_regen){
            if(later){
                printf(", ");
            }
            printf("hp regeneration = %d", item_temp.hp_regen);
            later = 1;
        }
        if(item_temp.mp){
            if(later){
                printf(", ");
            }
            printf("mp = %d", item_temp.mp);
            later = 1;
        }
        if(item_temp.mp_regen){
            if(later){
                printf(", ");
            }
            printf("mp regeneration = %d", item_temp.mp_regen);
            later = 1;
        }
        printf(")");

        if(item_temp.slots == 32){
            if(player.equipment[5] == i || player.equipment[6] == i){
                printf(" X");
            }
        }else{
            for(int j = 0; j < 5; j++){
                if(item_temp.slots & 1 << j && player.equipment[j] == i){
                    printf(" X");
                    break;
                }
            }
        }

        printf("\x1b[0m\n");
    }

    printf("\nSpels:\n");
    for(int i = 0; i < player.inventory.spels_amount; i++){
        if(type == 1 && index == i){
            printf("\x1b[31m");
        }
        printf("%2d: %s\x1b[0m\n", i + 1, player.inventory.spels[i].name);

    }
    printf("\nConsumables:\n");
    for(int i = 0; i < player.inventory.consumables_amount; i++){
        if(type == 2 && index == i){
            printf("\x1b[31m");
        }
        printf("%2d: %s (", i + 1, player.inventory.consumables[i].name);

        if(player.inventory.consumables[i].hp){
            printf("hp = %d", player.inventory.consumables[i].hp);
        }
        if(player.inventory.consumables[i].hp && player.inventory.consumables[i].mp){
            printf(", ");
        }
        if(player.inventory.consumables[i].mp){
            printf("mp = %d", player.inventory.consumables[i].mp);
        }
        printf(")\x1b[0m\n");

    }

    printf("\nNavigate W, S,   ");
    if(type == 0){
        printf("Drop item D");
    }
    printf("\n");

}

void uppdate_player_stats(){
    player.max_hp = 10;
    player.max_mp = 0;
    player.hp_regen = 0;
    player.mp_regen = 0;
    player.damage = 1;
    player.defense = 0;

    for(int i = 0; i < 7; i++){
        if(player.equipment[i] == -1){
            continue;
        }
        int test = 0;
        for(int j = 0; j < i; j++){
            if(player.equipment[j] == player.equipment[i]){
                test = 1;
                break;
            }
        }
        if(test){
            continue;
        }
        player.max_hp   += player.inventory.items[player.equipment[i]].hp;
        player.max_mp   += player.inventory.items[player.equipment[i]].mp;
        player.hp_regen += player.inventory.items[player.equipment[i]].hp_regen;
        player.mp_regen += player.inventory.items[player.equipment[i]].mp_regen;
        player.damage   += player.inventory.items[player.equipment[i]].attack;
        player.defense  += player.inventory.items[player.equipment[i]].defense;
    }

    if(player.hp > player.max_hp){
        player.hp = player.max_hp;
    }
    if(player.mp > player.max_mp){
        player.mp = player.max_mp;
    }

}

void use_item(int index){
    items_t item_temp = player.inventory.items[index];

    int exit = 0;

    if(item_temp.slots == 32){
        if(player.equipment[5] == index){
            player.equipment[5] = -1;
            return;
        }
        if(player.equipment[6] == index){
            player.equipment[6] = -1;
            return;
        }

        if(player.equipment[5] == -1){
            player.equipment[5] = index;
            return;
        }
        if(player.equipment[6] == -1){
            player.equipment[6] = index;
        }
        return;
    }

    for(int i = 0; i < 5; i++){
        if(item_temp.slots & 1 << i){
            if(player.equipment[i] != -1){
                if(player.equipment[i] == index){
                    player.equipment[i]  = -1;
                }
                exit = 1;
            }
        }
    }

    if(exit){
        return;
    }

    for(int i = 0; i < 5; i++){
        if(item_temp.slots & 1 << i){
            player.equipment[i] = index;
        }
    }
}

void remove_item(int i){
    for(int j = 0; j < 7; j++){
        if(player.equipment[j] > i){
            player.equipment[j]--;
        }
    }

    for(i++; i < inventory_size; i++){
        player.inventory.items[i - 1] = player.inventory.items[i];
    }
    player.inventory.items_amount--;
}

void use_spel(int index){
    int mode = 1;
    int target_x = player.x;
    int target_y = player.y;
    int i = 0;
    begining:;
    if(mode){
        for(; i <= aktive_enemies; i++){
            if(map3[enemies[i].x][enemies[i].y] && enemies[i].hp > 0){
                map4[enemies[i].x][enemies[i].y] = 2;
                break;
            }
            if(i == aktive_enemies){
                mode = 0;
                goto begining;
            }
        }
        char c;

        while(1){
            printf("\x1b[f");
            draw_map();
            c = getchar();
            switch(c){
                case 'a':
                case 'A':{
                    map4[enemies[i].x][enemies[i].y] = 0;
                    i++;
                    for(; map3[enemies[i].x][enemies[i].y] == 0 || enemies[i].hp <= 0; i++){
                        if(i >= aktive_enemies){
                            i = 0;
                        }
                    }
                    map4[enemies[i].x][enemies[i].y] = 2;
                }break;

                case 'd':
                case 'D':{
                    map4[enemies[i].x][enemies[i].y] = 0;
                    i--;
                    for(; map3[enemies[i].x][enemies[i].y] == 0 || enemies[i].hp <= 0; i--){
                        if(i < 0){
                            i = aktive_enemies;
                        }
                    }
                    map4[enemies[i].x][enemies[i].y] = 2;
                }break;

                case '\n':
                    target_x = enemies[i].x;
                    target_y = enemies[i].y;
                    goto exit;

                case 'e':
                case 'E':
                    return;

                case 'm':
                case 'M':
                    map4[enemies[i].x][enemies[i].y] = 0;
                    mode = 0;
                    goto begining;
            }
        }
    }else{
        while(1){
            printf("\x1b[f");
            draw_map();
            char c = getchar();
            int dir = 0;
            switch(c){
                case 'w':
                case 'W':
                    dir = upp;
                    break;
                
                case 'a':
                case 'A':
                    dir = left;
                    break;
                
                case 's':
                case 'S':
                    dir = down;
                    break;

                case 'd':
                case 'D':
                    dir = right;
                    break;

                case '\n':
                    goto exit;

                case 'e':
                case 'E':
                    return;

                case 'm':
                case 'M':
                    map4[target_x][target_y] = 0;
                    mode = 1;
                    goto begining;

                default:
                    continue;
            }

            if(map3[target_x + spatern_x[dir]][target_y + spatern_y[dir]] == 1){
                map4[target_x][target_y] = 0;
                target_x += spatern_x[dir];
                target_y += spatern_y[dir];
                map4[target_x][target_y] = 2;
            }

        }
    }
    exit:;

    switch(player.inventory.spels[index].type){
        case 0:{
            float disten = distens(target_x, target_y, player.x, player.y);
            float dir_x = (target_x - player.x) / disten;
            float dir_y = (target_y - player.y) / disten;
            dir_x *= 2;
            dir_y *= 2;
            dir_x = round(dir_x);
            dir_y = round(dir_y);
            bolt_spawn(player.x , player.y, dir_x, dir_y, player.damage);
        }break;
    }
}

void use_consumables(int index){
    player.hp += player.inventory.consumables[index].hp;
    player.mp += player.inventory.consumables[index].mp;

    if(player.hp > player.max_hp){
        player.hp = player.max_hp;
    }
    if(player.mp > player.max_mp){
        player.mp = player.max_mp;
    }

    index++;
    for(; index < inventory_size; index++){
        player.inventory.consumables[index - 1] = player.inventory.consumables[index];
        printf("%d\n", index);
    }
    player.inventory.consumables_amount--;
}

void inventory_run(){
    int marker_type = 0;
    int marker_index = 0;
    if(player.inventory.items_amount == 0){
        marker_type = 1;
    }
    if(player.inventory.spels_amount == 0 && marker_type == 1){
        marker_type = 2;
    }
    if(player.inventory.consumables_amount == 0 && marker_type == 2){
        return;
    }
    while(1){
        draw_inventory(marker_type, marker_index);
        int c = getchar();
        switch(c){
            case 's':
            case 'S':{
                marker_index++;
                if(marker_type == 0 && marker_index == player.inventory.items_amount){
                    marker_index = 0;
                    marker_type++;
                }
                if(marker_type == 1 && marker_index == player.inventory.spels_amount){
                    if(player.inventory.consumables_amount == 0){
                        marker_index = player.inventory.spels_amount - 1;
                        if(marker_index == -1){
                            marker_type = 0;
                            marker_index = player.inventory.items_amount -1;
                        }
                    }else{
                        marker_index = 0;
                        marker_type++;
                    }
                }
                if(marker_type == 2 && marker_index == player.inventory.consumables_amount){
                    marker_index--;
                }
            }break;

            case 'w':
            case 'W':{
                marker_index--;
                if(marker_type == 2 && marker_index == -1){
                    marker_index = player.inventory.spels_amount - 1;
                    marker_type--;
                }
                if(marker_type == 1 && marker_index == -1){
                    if(player.inventory.items_amount == 0){
                        marker_index = 0;
                        if(marker_index == player.inventory.spels_amount){
                            marker_type = 2;
                            marker_index = 0;
                        }
                    }else{
                        marker_index = player.inventory.items_amount - 1;
                        marker_type--;
                    }
                }
                if(marker_type == 0 && marker_index == -1){
                    marker_index = 0;
                }
            }break;

            case 'd':
            case 'D':{
                if(marker_type != 0){
                    break;
                }
                int exit = 0;
                for(int i = 0; i < 7; i++){
                    if(player.equipment[i] == marker_index){
                        exit = 1;
                    }
                }
                if(exit){
                    break;
                }
                if(conformation("remove this item")){
                    remove_item(marker_index);
                    return;
                }
            }break;

            case '\n':
                switch(marker_type){
                    case 0:
                        use_item(marker_index);
                        uppdate_player_stats();
                        break;
                    case 1:
                        use_spel(marker_index);
                        break;
                    case 2:
                        use_consumables(marker_index);
                        break;
                }
                printf("\x1b[1;1H\x1b[2J");
            return;
            
            case 'e':
            case 'E':
                printf("\x1b[1;1H\x1b[2J");
                return;
        }
    }
}

int move_player(){
    //map movment
    int uppdate = 0;
    char c;
    int dir = -1;
    c = getchar();
    printf("\x1b[F \b");
    switch(c){
        case 'w':
        case 'W':
            dir = upp;
            break;
        
        case 's':
        case 'S':
            dir = down;
            break;
        
        case 'a':
        case 'A':
            dir = left;
            break;
        
        case 'd':
        case 'D':
            dir = right;
            break;

        case 'e':
        case 'E':
            if(conformation("exit the game")){
                return -1;
            }
            return 0;
        
        case ' ':
            return 1;
        
        case 'm':
        case 'M':
            if(map_mode == map_normal){
                map_mode = map_world;
            }else{
                map_mode = map_normal;
            }
            printf("\x1b[1;1H\x1b[2J");
            return 1;
        
        case 'i':
        case 'I':
            inventory_run();
            return 1;


        default:
            return 0;
    }

    if(map_mode == map_normal){
        if(map2[player.x + spatern_x[dir]][player.y + spatern_y[dir]] != 0 && !(map2[player.x + spatern_x[dir]][player.y + spatern_y[dir]] & all_flages)){
            int i;
            for(i = 0; i < aktive_enemies; i++){
                if(enemies[i].x == player.x + spatern_x[dir] && enemies[i].y == player.y + spatern_y[dir] && enemies[i].hp > 0){
                    break;
                }
            }
            enemies[i].hp -= player.damage;
            if(enemies[i].hp <= 0){
                player.hp += 6;
                kill_enemie(i);
            }
            uppdate = 1;
        }else if(!is_solid(player.x + spatern_x[dir], player.y + spatern_y[dir])){
            player.x += spatern_x[dir];
            player.y += spatern_y[dir];
            uppdate = 1;

            if(map2[player.x][player.y] & consumables_flag){
                if(player.inventory.consumables_amount < max_items){
                    int i = player.inventory.consumables_amount;
                    int j = map2[player.x][player.y] & ~all_flages;
                    player.inventory.consumables[i] = consumables[j];
                    consumables[j] = (consumables_t){ 0 };
                    player.inventory.consumables_amount++;
                }
            }else if(map2[player.x][player.y] & item_flag){
                if(player.inventory.items_amount < max_items){
                    int i = player.inventory.items_amount;
                    int j = map2[player.x][player.y] & ~all_flages;
                    player.inventory.items[i] = items[j];
                    items[j] = (items_t){ 0 };
                    player.inventory.items_amount++;
                }
            }
        }

        //world movment
        if(map1[player.x][player.y] == tile_exit){
            int dir;
            if(player.x == 0){
                world_pos_x--;
                dir = 3;
            }else if(player.x == width - 1){
                world_pos_x++;
                dir = 2;
            }else if(player.y == 0){
                world_pos_y--;
                dir = 1;
            }else if(player.y == height - 1){
                world_pos_y++;
                dir = 0;
            }
            map_gen();
            player.x = world_map[world_pos_x][world_pos_y].exits_x[dir];
            player.y = world_map[world_pos_x][world_pos_y].exits_y[dir];
            uppdate = 1;
        }
    }

    return uppdate;
}

//visebilytty 
float slope(int x, int y){
    return (y - 0.5f) / x;
}

void transform(int x, int y, int* out_x, int* out_y){
    switch(player.dir){
        case left:
            *out_x = player.x - x;
            *out_y = player.y + y;
            break;
        case right:
            *out_x = player.x + x;
            *out_y = player.y + y;
            break;
        case upp:
            *out_x = player.x + y;
            *out_y = player.y - x;
            break;
        case down:
            *out_x = player.x + y;
            *out_y = player.y + x;
            break;
    }
}

int tiles(ROW row, int* out_x, int* out_y){
    int min_col = floor((row.deapth * row.start_slope) + 0.5f);
    int max_col = ceil((row.deapth * row.end_slope) - 0.5f);
    for(int i = 0; i <= max_col - min_col; i++){
        out_x[i] = row.deapth;
        out_y[i] = min_col + i;
    }
    return max_col - min_col + 1;
}

void reveal_tile(int x, int y){
    int true_x;
    int true_y;

    transform(x, y, &true_x, &true_y);
    if(true_x < 0 || true_x > width - 1 || true_y < 0 || true_y > height - 1){
        return;
    }
    map3[true_x][true_y] = 1;
}

int is_symmetric(ROW row, int x, int y){
    return (y >= row.deapth * row.start_slope && y <= row.deapth * row.end_slope);
}

int get_tile(int x, int y){
    if(x == -1 && y == -1){
        return 0;
    }
    int true_x;
    int true_y;

    transform(x, y, &true_x, &true_y);
    if(true_x < 0 || true_x > width - 1 || true_y < 0 || true_y > height - 1){
        return 1;
    }
    if(map1[true_x][true_y] == tile_rock || map1[true_x][true_y] == tile_hardrock || map1[true_x][true_y] == tile_wall || map1[true_x][true_y] == tile_door){
        return 1;
    }
    if(map1[true_x][true_y] == tile_floor || map1[true_x][true_y] == tile_exit){
        return 0;
    }
    return 0;
}

void scan(ROW row){
    int tile_x[100];
    int tile_y[100];

    int max = tiles(row, tile_x, tile_y);
    int prev_tile = -1;

    for(int i = 0; i < max; i++){
        int tile = get_tile(tile_x[i], tile_y[i]);

        if((tile == 1 || is_symmetric(row, tile_x[i], tile_y[i])) && tile != -1){
            reveal_tile(tile_x[i], tile_y[i]);
        }
        if((prev_tile == 1 && tile == 0) && !(prev_tile == -1 || tile == -1)){
            row.start_slope = slope(tile_x[i], tile_y[i]);
        }
        if((prev_tile == 0 && tile == 1) && !(prev_tile == -1 || tile == -1)){
            ROW next_row = {.deapth = row.deapth + 1, .start_slope = row.start_slope, .end_slope = slope(tile_x[i], tile_y[i])};
            scan(next_row);
        }
        prev_tile = tile;
    }
    if(prev_tile == 0){
        ROW next_row = {.deapth = row.deapth + 1, .start_slope = row.start_slope, .end_slope = row.end_slope};
        scan(next_row);
    }
}


int main(){
    printf("\x1b[?1049h");

    static struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    setlocale(LC_ALL, "en_US.UTF-8");

    for(int i = 0; i < 7; i++){
        player.equipment[i] = -1;
    }
    uppdate_player_stats();

    world_map_gen();

    map_gen();

    plase_player();

    printf("\x1b[f");
    for(int i = 0; i < search_cross; i++){
        player.dir = i;
        ROW first_row = {.deapth = 1, .start_slope = -1, .end_slope = 1};
        scan(first_row);
    }
    draw_map();
    printf("\x1b[f");

    while(player.hp > 0){
        int action = move_player();
        if(action == -1){
            break;
        }else if(action){
            clear_information();
            if(map_mode == map_normal){
                for(int i = 0; i < search_cross; i++){
                    player.dir = i;
                    ROW first_row = {.deapth = 1, .start_slope = -1, .end_slope = 1};
                    scan(first_row);
                }
                enemies_uppdete();
                bolt_uppdate();
                explotion();
            }
            plase_items();
            draw_map();
            printf("\x1b[f");
        }
    }
    if(player.hp <=0){
        printf("\x1b[1;1H\e[2J\x1b[31mGame Over\x1b[0m\nPress enter to quit\n");

        while(getchar() != '\n');
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    printf("\x1b[?1049l");

    return 0;
}