#include <iostream>
#include <string>
#include <ctime>
#include <vector>    // 用於儲存多個障礙物
#include <algorithm> // 用於 std::max
#include "rlutil.h"

using namespace std;

// --- 遊戲設定 ---
const int TRACK_WIDTH = 22;   // 賽道寬度
const int SCREEN_HEIGHT = 20; // 畫面高度
const int INPUT_DELAY = 30;   // 輸入偵測頻率 (毫秒) - 越小越靈敏
const int MAX_OBSTACLES = 5;  // 畫面上最多同時幾個障礙物

// 定義障礙物結構
struct Obstacle {
    int x;
    int y;
    int dir; // 0:直線, -1:左移, 1:右移
};

int main() {
    // 1. 初始化環境
    rlutil::saveDefaultColor();
    rlutil::cls();             
    rlutil::hidecursor();      
    srand(time(NULL));

    // 2. 變數定義
    int playerX = TRACK_WIDTH / 2;
    int score = 0;
    int level = 1;
    bool gameOver = false;
    
    // 遊戲節奏控制
    int gameSpeedDelay = 5; // 數值越小，賽道滾動越快
    int frameCounter = 0;   // 計數器
    
    // 多重障礙物容器
    vector<Obstacle> obstacles;

    // 計算螢幕中心
    int startX = (rlutil::tcols() - TRACK_WIDTH) / 2; 

    // 3. 遊戲主迴圈
    while (!gameOver) {
        
        // --- A. 輸入處理 (高頻率偵測，讓長按更順暢) ---
        if (kbhit()) {
            int key = rlutil::getkey();
            if (key == rlutil::KEY_LEFT && playerX > 2) {
                playerX--; 
            } else if (key == rlutil::KEY_RIGHT && playerX < TRACK_WIDTH - 1) {
                playerX++;
            } else if (key == rlutil::KEY_ESCAPE) {
                break;
            }
        }

        // --- B. 遊戲環境更新 (低頻率，控制速度) ---
        // 只有當計數器達到設定值時，賽道才滾動
        if (frameCounter >= gameSpeedDelay) {
            frameCounter = 0; // 重置計數器

            // 1. 移動所有現存障礙物
            for (int i = 0; i < obstacles.size(); i++) {
                obstacles[i].y++; // 向下掉

                // 左右移動邏輯 (等級2以上)
                if (level > 1 && obstacles[i].dir != 0) {
                    obstacles[i].x += obstacles[i].dir;
                    // 撞牆反彈
                    if (obstacles[i].x <= 1) { obstacles[i].x = 2; obstacles[i].dir = 1; }
                    if (obstacles[i].x >= TRACK_WIDTH) { obstacles[i].x = TRACK_WIDTH - 1; obstacles[i].dir = -1; }
                }
            }

            // 2. 移除超出畫面的障礙物 & 增加分數
            // 使用迭代器從後往前檢查，方便刪除
            for (int i = obstacles.size() - 1; i >= 0; i--) {
                if (obstacles[i].y > SCREEN_HEIGHT) {
                    obstacles.erase(obstacles.begin() + i); // 從清單移除
                    score++;
                    
                    // 升級邏輯：每過 10 個障礙物升一級，速度變快
                    if (score % 10 == 0) {
                        level++;
                        // 讓 gameSpeedDelay 變小 (最小值為 1)
                        gameSpeedDelay = (std::max)(1, 5 - (level / 2));
                    }
                }
            }

            // 3. 生成新障礙物 (如果畫面上的障礙物還不夠多)
            // 邏輯：最後一個障礙物必須已經掉落一段距離，才生成下一個 (避免黏在一起)
            bool canSpawn = true;
            if (!obstacles.empty()) {
                if (obstacles.back().y < 4) canSpawn = false; // 間距至少 4 格
            }

            if (canSpawn && obstacles.size() < MAX_OBSTACLES) {
                Obstacle newObs;
                newObs.x = rand() % (TRACK_WIDTH - 4) + 3;
                newObs.y = 1;
                
                // 決定移動模式
                if (level >= 2 && (rand() % 100) < (level * 10)) {
                    newObs.dir = (rand() % 2 == 0) ? 1 : -1;
                } else {
                    newObs.dir = 0;
                }
                obstacles.push_back(newObs);
            }
        }

        // --- C. 碰撞檢測 (每一幀都要檢查) ---
        for (const auto& obs : obstacles) {
            if (obs.y == SCREEN_HEIGHT - 1 && playerX == obs.x) {
                gameOver = true;
            }
        }

        // --- D. 畫面繪製 ---
        
        // 繪製賽道框線
        for (int y = 1; y <= SCREEN_HEIGHT; y++) {
            rlutil::locate(startX, y);
            rlutil::setColor(rlutil::WHITE); cout << "|";
            rlutil::locate(startX + TRACK_WIDTH + 1, y);
            cout << "|";
            
            // 清除賽道內部
            rlutil::locate(startX + 1, y);
            cout << string(TRACK_WIDTH, ' '); 
        }

        // 繪製所有障礙物
        for (const auto& obs : obstacles) {
            if (obs.y <= SCREEN_HEIGHT && obs.y >= 1) {
                rlutil::locate(startX + obs.x, obs.y);
                rlutil::setColor(rlutil::LIGHTRED);
                cout << (obs.dir == 0 ? "O" : (obs.dir == 1 ? ">" : "<"));
            }
        }

        // 繪製玩家
        rlutil::locate(startX + playerX, SCREEN_HEIGHT - 1);
        rlutil::setColor(rlutil::LIGHTBLUE);
        if (gameOver) {
            rlutil::setColor(rlutil::RED); cout << "X";
        } else {
            cout << "A";
        }

        // 繪製 UI
        rlutil::locate(1, 2);
        rlutil::setColor(rlutil::YELLOW); cout << "Score: " << score;
        rlutil::locate(1, 3);
        rlutil::setColor(rlutil::CYAN);   cout << "Level: " << level;

        // --- E. 迴圈控制 ---
        rlutil::msleep(INPUT_DELAY);
        frameCounter++; // 增加計數器
    }

    // 4. 遊戲結束
    rlutil::locate(startX + 2, SCREEN_HEIGHT / 2);
    rlutil::setBackgroundColor(rlutil::RED);
    rlutil::setColor(rlutil::WHITE);
    cout << " GAME OVER ";
    
    rlutil::resetColor();
    rlutil::showcursor();
    rlutil::locate(1, SCREEN_HEIGHT + 2);
    rlutil::anykey("Press any key to exit...");

    return 0;
}