#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
#include <math.h>

using namespace std;

// 隐藏控制台窗口
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

#define WINDOW_WIDTH		480			// 窗口宽度
#define WINDOW_HEIGHT		700			// 窗口高度
#define MOVE_SPEED			15			// 英雄战机移动速度
#define HERO_BULLET_SPEED	10			// 英雄战机子弹移动速度
#define ENEMY_BULLET_SPEED	2			// 敌人战机子弹移动速度
#define HERO_BULLET_INTERVAL	20		// 英雄战机发射子弹的帧间隔
#define HERO_ANIM_SPEED			5		// 英雄战机动画效果帧间隔
#define BACKIMG_SPEED			3		// 背景图片移动速度

#define ENEMY_GENE_MIN_INTERVAL		75		// 敌人战机生成最小帧间隔
#define ENEMY_DESTORY_ANIM_DELAY	5		// 敌人战机坠毁动画效果帧间隔

#define ENEMY_EASY_SPEED	2	// 简单的敌人战机移动速度
#define ENEMY_MEDIUM_SPEED	2	// 中等的敌人战机移动速度
#define ENEMY_BOSS_SPEED	1	// 困难的敌人战机移动速度

#define ENEMY_EASY_LIFE		2	// 简单的敌人战机生命值
#define ENEMY_MEDIUM_LIFE	5	// 中等的敌人战机生命值
#define ENEMY_BOSS_LIFE		10	// 困难的敌人战机生命值

#define ENEMY_EASY_BULLET_INTERVAL		200	// 简单的敌人战机发射子弹帧间隔
#define ENEMY_MEDIUM_BULLET_INTERVAL	250	// 中等的敌人战机发射子弹帧间隔
#define ENEMY_BOSS_BULLET_INTERVAL		400	// 困难的敌人战机发射子弹帧间隔

#define ENTRY_DIRECTION_TOP		0
#define ENTRY_DIRECTION_LEFT	1
#define ENTRY_DIRECTION_RIGHT	2

#define ENEMY_TYPE_EASY		0
#define ENEMY_TYPE_MEDIUM	1
#define ENEMY_TYPE_BOSS		2

typedef struct Bullet
{
	SDL_Rect rect;	// 子弹位置及缩放矩形
	int speed = 0;	// 子弹速度
};

typedef struct Enemy
{
	int timerBullet = 0;					// 敌人战机子弹发射时间定时器
	int timerDestory = 0;					// 敌人战机坠毁动画定时器
	int life = 0;							// 敌人战机当前生命值
	int direction;							// 敌人战机进入战场的方向
	int type;								// 敌人战机类型
	SDL_Rect rect;							// 敌人战机位置及缩放矩形
	vector<SDL_Texture*> destoryAnimList;	// 敌人战机坠毁动画Texture向量
	int animIndex = 0;						// 敌人战机坠毁动画Texture下标
};

int timerHeroBullet = 0;	// 英雄战机子弹发射间隔定时器
int timerHeroImg = 0;		// 英雄战机更换动画效果定时器
int timerEnemyGenerate = 0;	// 敌人战机生成间隔定时器
int cutYCoord = 0;			// 实现动态背景图的裁剪Y坐标

vector<Bullet> vHeroBullets;	// 英雄战机子弹向量
vector<Bullet> vEnemyBullets;	// 敌人战机子弹向量
vector<Enemy> vEnemies;			// 敌人向量

SDL_Rect rectHero;

SDL_Surface* surHeroNormal_1;
SDL_Surface* surHeroNormal_2;
SDL_Surface* surEnemyEasy_1;
SDL_Surface* surEnemyEasy_2;
SDL_Surface* surEnemyEasy_3;
SDL_Surface* surEnemyEasy_4;
SDL_Surface* surEnemyEasy_5;
SDL_Surface* surEnemyMedium_1;
SDL_Surface* surEnemyMedium_2;
SDL_Surface* surEnemyMedium_3;
SDL_Surface* surEnemyMedium_4;
SDL_Surface* surEnemyMedium_5;
SDL_Surface* surEnemyBoss_1;
SDL_Surface* surEnemyBoss_2;
SDL_Surface* surEnemyBoss_3;
SDL_Surface* surEnemyBoss_4;
SDL_Surface* surEnemyBoss_5;
SDL_Surface* surEnemyBoss_6;
SDL_Surface* surEnemyBoss_7;
SDL_Surface* surBackground;
SDL_Surface* surBulletHero;
SDL_Surface* surBulletEnemy;

SDL_Texture* texCurrentHero;	// 当前英雄战机的Texture
SDL_Texture* texHeroNormal_1;
SDL_Texture* texHeroNormal_2;
SDL_Texture* texEnemyEasy_1;
SDL_Texture* texEnemyEasy_2;
SDL_Texture* texEnemyEasy_3;
SDL_Texture* texEnemyEasy_4;
SDL_Texture* texEnemyEasy_5;
SDL_Texture* texEnemyMedium_1;
SDL_Texture* texEnemyMedium_2;
SDL_Texture* texEnemyMedium_3;
SDL_Texture* texEnemyMedium_4;
SDL_Texture* texEnemyMedium_5;
SDL_Texture* texEnemyBoss_1;
SDL_Texture* texEnemyBoss_2;
SDL_Texture* texEnemyBoss_3;
SDL_Texture* texEnemyBoss_4;
SDL_Texture* texEnemyBoss_5;
SDL_Texture* texEnemyBoss_6;
SDL_Texture* texEnemyBoss_7;
SDL_Texture* texBackground;
SDL_Texture* texBulletHero;
SDL_Texture* texBulletEnemy;

bool isDeleteHeroBullet(Bullet bullet)
{
	if (bullet.rect.y <= 0)
	{
		return true;
	}
	else
	{
		int decisionPointX = bullet.rect.x + bullet.rect.w / 2;
		int decisionPointY = bullet.rect.y;
		bool isCoolide = false;

		cout << vEnemies.size() << endl;
		for (auto& enemy : vEnemies)
		{
			if ((decisionPointX >= enemy.rect.x)
				&& (decisionPointX <= enemy.rect.x + enemy.rect.w)
				&& (decisionPointY >= enemy.rect.y)
				&& (decisionPointY <= enemy.rect.y + enemy.rect.h))
			{
				isCoolide = true;
				//cout << "COOLIDE" << endl;
				if (enemy.life > 0)
				{
					enemy.life--;
				}
				break;
			}
		}

		return isCoolide;
	}
}

bool isDeleteEnemyBullet(Bullet bullet)
{
	if (bullet.rect.y >= WINDOW_HEIGHT)
	{
		return true;
	}
	else
	{
		int decisionPointX = bullet.rect.x + bullet.rect.w / 2;
		int decisionPointY = bullet.rect.y + bullet.rect.h;
		return (decisionPointX >= rectHero.x)
			&& (decisionPointX <= rectHero.x + rectHero.w)
			&& (decisionPointY >= rectHero.y)
			&& (decisionPointY <= rectHero.y + rectHero.h);
	}
}

bool isDeleteEnemy(Enemy enemy)
{
	return enemy.animIndex >= enemy.destoryAnimList.size() - 1;
}

void fireHeroBullets(SDL_Renderer* rend)
{
	int ratio = 2;	// 子弹图片缩放比率
	Bullet m_leftBullet;
	Bullet m_rightBullet;

	m_leftBullet.speed = HERO_BULLET_SPEED;
	m_rightBullet.speed = HERO_BULLET_SPEED;

	m_leftBullet.rect.x = rectHero.x + 15;
	m_leftBullet.rect.y = rectHero.y + 20;
	m_leftBullet.rect.w = surBulletHero->w * ratio;
	m_leftBullet.rect.h = surBulletHero->h * ratio;
	m_rightBullet.rect.x = rectHero.x + surHeroNormal_1->w - 23;
	m_rightBullet.rect.y = rectHero.y + 20;
	m_rightBullet.rect.w = surBulletHero->w * ratio;
	m_rightBullet.rect.h = surBulletHero->h * ratio;
	SDL_RenderCopy(rend, texBulletHero, NULL, &m_leftBullet.rect);
	SDL_RenderCopy(rend, texBulletHero, NULL, &m_rightBullet.rect);

	vHeroBullets.push_back(m_leftBullet);
	vHeroBullets.push_back(m_rightBullet);
}

void updateHeroBullets(SDL_Renderer* rend)
{
	// 销毁已经飞出屏幕外或击中敌人的子弹
	vHeroBullets.erase(remove_if(vHeroBullets.begin(), vHeroBullets.end(), isDeleteHeroBullet), vHeroBullets.end());

	for (auto& bullet : vHeroBullets)
	{
		bullet.rect.y -= bullet.speed;
		SDL_RenderCopy(rend, texBulletHero, NULL, &bullet.rect);
	}
}

// 生成敌人战机
void generateEnemy(SDL_Renderer* rend)
{
	srand(time(0));

	Enemy enemy;
	int typeRandNum = rand() % 100;
	if (typeRandNum <= 60)
	{
		enemy.type = ENEMY_TYPE_EASY;
		enemy.life = ENEMY_EASY_LIFE;
		enemy.destoryAnimList.push_back(texEnemyEasy_2);
		enemy.destoryAnimList.push_back(texEnemyEasy_3);
		enemy.destoryAnimList.push_back(texEnemyEasy_4);
		enemy.destoryAnimList.push_back(texEnemyEasy_5);
	}
	else if (typeRandNum <= 90)
	{
		enemy.type = ENEMY_TYPE_MEDIUM;
		enemy.life = ENEMY_MEDIUM_LIFE;
		enemy.destoryAnimList.push_back(texEnemyMedium_2);
		enemy.destoryAnimList.push_back(texEnemyMedium_3);
		enemy.destoryAnimList.push_back(texEnemyMedium_4);
		enemy.destoryAnimList.push_back(texEnemyMedium_5);
	}
	else
	{
		enemy.type = ENEMY_TYPE_BOSS;
		enemy.life = ENEMY_BOSS_LIFE;
		enemy.destoryAnimList.push_back(texEnemyBoss_2);
		enemy.destoryAnimList.push_back(texEnemyBoss_3);
		enemy.destoryAnimList.push_back(texEnemyBoss_4);
		enemy.destoryAnimList.push_back(texEnemyBoss_5);
		enemy.destoryAnimList.push_back(texEnemyBoss_6);
		enemy.destoryAnimList.push_back(texEnemyBoss_7);
	}

	enemy.direction = rand() % 3;
	 
	if (enemy.type == ENEMY_TYPE_EASY)
	{
		switch (enemy.direction)
		{
		case ENTRY_DIRECTION_TOP:
			enemy.rect.x = rand() % (WINDOW_WIDTH - surEnemyEasy_1->w);
			enemy.rect.y = 0 - surEnemyEasy_1->h;
			break;
		case ENTRY_DIRECTION_LEFT:
			enemy.rect.x = 0 - surEnemyEasy_1->w;
			enemy.rect.y = rand() % int(WINDOW_HEIGHT * 0.5 - surEnemyEasy_1->h);
			break;
		case ENTRY_DIRECTION_RIGHT:
			enemy.rect.x = WINDOW_WIDTH;
			enemy.rect.y = rand() % int(WINDOW_HEIGHT * 0.5 - surEnemyEasy_1->h);
			break;
		}
		enemy.rect.w = surEnemyEasy_1->w;
		enemy.rect.h = surEnemyEasy_1->h;
		SDL_RenderCopy(rend, texEnemyEasy_1, NULL, &enemy.rect);
	}
	else if (enemy.type == ENEMY_TYPE_MEDIUM)
	{
		switch (enemy.direction)
		{
		case ENTRY_DIRECTION_TOP:
			enemy.rect.x = rand() % (WINDOW_WIDTH - surEnemyMedium_1->w);
			enemy.rect.y = 0 - surEnemyMedium_1->h;
			break;
		case ENTRY_DIRECTION_LEFT:
			enemy.rect.x = 0 - surEnemyMedium_1->w;
			enemy.rect.y = rand() % int(WINDOW_HEIGHT * 0.5 - surEnemyMedium_1->h);
			break;
		case ENTRY_DIRECTION_RIGHT:
			enemy.rect.x = WINDOW_WIDTH;
			enemy.rect.y = rand() % int(WINDOW_HEIGHT * 0.5 - surEnemyMedium_1->h);
			break;
		}
		enemy.rect.w = surEnemyMedium_1->w;
		enemy.rect.h = surEnemyMedium_1->h;
		SDL_RenderCopy(rend, texEnemyMedium_1, NULL, &enemy.rect);
	}
	else
	{
		switch (enemy.direction)
		{
		case ENTRY_DIRECTION_TOP:
			enemy.rect.x = rand() % (WINDOW_WIDTH - surEnemyBoss_1->w);
			enemy.rect.y = 0 - surEnemyBoss_1->h;
			break;
		case ENTRY_DIRECTION_LEFT:
			enemy.rect.x = 0 - surEnemyBoss_1->w;
			enemy.rect.y = rand() % int(WINDOW_HEIGHT * 0.5 - surEnemyBoss_1->h);
			break;
		case ENTRY_DIRECTION_RIGHT:
			enemy.rect.x = WINDOW_WIDTH;
			enemy.rect.y = rand() % int(WINDOW_HEIGHT * 0.5 - surEnemyBoss_1->h);
			break;
		}
		enemy.rect.w = surEnemyBoss_1->w;
		enemy.rect.h = surEnemyBoss_1->h;
		SDL_RenderCopy(rend, texEnemyBoss_1, NULL, &enemy.rect);
	}

	vEnemies.push_back(enemy);
}

void fireEnemiesBullets(SDL_Renderer* rend)
{
	for (auto &enemy : vEnemies)
	{
		switch (enemy.type)
		{
		case ENEMY_TYPE_EASY:
			if (enemy.timerBullet >= ENEMY_EASY_BULLET_INTERVAL)
			{
				int ratio = 2;	// 子弹图片缩放比率
				Bullet bullet;
				bullet.speed = ENEMY_BULLET_SPEED;
				bullet.rect.w = surBulletEnemy->w * ratio;
				bullet.rect.h = surBulletEnemy->h * ratio;
				bullet.rect.x = enemy.rect.x + surEnemyEasy_1->w / 2;
				bullet.rect.y = enemy.rect.y + surEnemyEasy_1->h;
				SDL_RenderCopy(rend, texBulletEnemy, NULL, &bullet.rect);
				vEnemyBullets.push_back(bullet);
				enemy.timerBullet = 0;
			}
			enemy.timerBullet++;
			break;
		case ENEMY_TYPE_MEDIUM:
			if (enemy.timerBullet >= ENEMY_MEDIUM_BULLET_INTERVAL)
			{
				int ratio = 2;	// 子弹图片缩放比率
				Bullet m_leftBullet;
				Bullet m_rightBullet;

				m_leftBullet.speed = ENEMY_BULLET_SPEED;
				m_rightBullet.speed = ENEMY_BULLET_SPEED;

				m_leftBullet.rect.x = enemy.rect.x + 5;
				m_leftBullet.rect.y = enemy.rect.y + 40;
				m_leftBullet.rect.w = surBulletEnemy->w * ratio;
				m_leftBullet.rect.h = surBulletEnemy->h * ratio;
				m_rightBullet.rect.x = enemy.rect.x + surEnemyEasy_1->w - 5;
				m_rightBullet.rect.y = enemy.rect.y + 40;
				m_rightBullet.rect.w = surBulletHero->w * ratio;
				m_rightBullet.rect.h = surBulletHero->h * ratio;
				SDL_RenderCopy(rend, texBulletEnemy, NULL, &m_leftBullet.rect);
				SDL_RenderCopy(rend, texBulletEnemy, NULL, &m_rightBullet.rect);

				vEnemyBullets.push_back(m_leftBullet);
				vEnemyBullets.push_back(m_rightBullet);

				enemy.timerBullet = 0;
			}
			enemy.timerBullet++;
			break;
		case ENEMY_TYPE_BOSS:
			if (enemy.timerBullet >= ENEMY_EASY_BULLET_INTERVAL)
			{
				int ratio = 4;	// 子弹图片缩放比率
				Bullet m_leftBullet;
				Bullet m_rightBullet;
				Bullet m_middleBullet_1;
				Bullet m_middleBullet_2;

				m_leftBullet.speed = ENEMY_BULLET_SPEED;
				m_rightBullet.speed = ENEMY_BULLET_SPEED;
				m_middleBullet_1.speed = ENEMY_BULLET_SPEED;
				m_middleBullet_2.speed = ENEMY_BULLET_SPEED;

				m_leftBullet.rect.w = surBulletEnemy->w * ratio;
				m_leftBullet.rect.h = surBulletEnemy->h * ratio;
				m_leftBullet.rect.x = enemy.rect.x + 5;
				m_leftBullet.rect.y = enemy.rect.y + 40;
				
				m_rightBullet.rect.w = surBulletEnemy->w * ratio;
				m_rightBullet.rect.h = surBulletEnemy->h * ratio;
				m_rightBullet.rect.x = enemy.rect.x + surEnemyBoss_1->w - 5;
				m_rightBullet.rect.y = enemy.rect.y + 40;
				
				m_middleBullet_1.rect.w = surBulletEnemy->w * ratio;
				m_middleBullet_1.rect.h = surBulletEnemy->h * ratio;
				m_middleBullet_1.rect.x = enemy.rect.x + surEnemyBoss_1->w / 2 - 10;
				m_middleBullet_1.rect.y = enemy.rect.y + surEnemyBoss_1->h;
				
				m_middleBullet_2.rect.w = surBulletEnemy->w * ratio;
				m_middleBullet_2.rect.h = surBulletEnemy->h * ratio;
				m_middleBullet_2.rect.x = enemy.rect.x + surEnemyBoss_1->w / 2 + 10;
				m_middleBullet_2.rect.y = enemy.rect.y + surEnemyBoss_1->h;

				SDL_RenderCopy(rend, texBulletEnemy, NULL, &m_leftBullet.rect);
				SDL_RenderCopy(rend, texBulletEnemy, NULL, &m_rightBullet.rect);
				SDL_RenderCopy(rend, texBulletEnemy, NULL, &m_middleBullet_1.rect);
				SDL_RenderCopy(rend, texBulletEnemy, NULL, &m_middleBullet_2.rect);

				vEnemyBullets.push_back(m_leftBullet);
				vEnemyBullets.push_back(m_rightBullet);
				vEnemyBullets.push_back(m_middleBullet_1);
				vEnemyBullets.push_back(m_middleBullet_2);

				enemy.timerBullet = 0;
			}
			enemy.timerBullet++;
			break;
		}
	}
}

void updateEnemiesBullets(SDL_Renderer* rend)
{
	// 销毁已经飞出屏幕外或击中英雄的子弹
	vEnemyBullets.erase(remove_if(vEnemyBullets.begin(), vEnemyBullets.end(), isDeleteEnemyBullet), vEnemyBullets.end());

	for (auto& bullet : vEnemyBullets)
	{
		bullet.rect.y += bullet.speed;
		SDL_RenderCopy(rend, texBulletEnemy, NULL, &bullet.rect);
	}
}

void updateEnmies(SDL_Renderer* rend)
{
	// 销毁已经被击毁且动画播放完成的敌人战机
	vEnemies.erase(remove_if(vEnemies.begin(), vEnemies.end(), isDeleteEnemy), vEnemies.end());

	for (auto &enemy : vEnemies)
	{
		// 如果当前敌人战机生命值为0，则代表其正在播放坠毁动画
		if (enemy.life == 0)
		{
			if (enemy.timerDestory >= ENEMY_DESTORY_ANIM_DELAY)
			{
				if (enemy.animIndex <= enemy.destoryAnimList.size() - 1)
				{
					SDL_RenderCopy(rend, enemy.destoryAnimList[enemy.animIndex], NULL, &enemy.rect);
				}
				enemy.animIndex++;
				enemy.timerDestory = 0;
			}
			enemy.timerDestory++;
		}
		else
		{
			switch (enemy.type)
			{
			case ENEMY_TYPE_EASY:
				switch (enemy.direction)
				{
				case ENTRY_DIRECTION_TOP:
					if (enemy.rect.y < WINDOW_HEIGHT * 0.5 - surEnemyEasy_1->h)
					{
						enemy.rect.y += ENEMY_EASY_SPEED;
					}
					else
					{
						enemy.direction = rand() % 2 == 0 ? ENTRY_DIRECTION_LEFT : ENTRY_DIRECTION_RIGHT;
					}
					break;
				case ENTRY_DIRECTION_LEFT:
					if (enemy.rect.x < WINDOW_WIDTH - surEnemyEasy_1->w)
					{
						enemy.rect.x += ENEMY_EASY_SPEED;
					}
					else
					{
						enemy.direction = ENTRY_DIRECTION_RIGHT;
					}
					break;
				case ENTRY_DIRECTION_RIGHT:
					if (enemy.rect.x > 0)
					{
						enemy.rect.x -= ENEMY_EASY_SPEED;
					}
					else
					{
						enemy.direction = ENTRY_DIRECTION_LEFT;
					}
					break;
				}
				SDL_RenderCopy(rend, texEnemyEasy_1, NULL, &enemy.rect);
				break;
			case ENEMY_TYPE_MEDIUM:
				switch (enemy.direction)
				{
				case ENTRY_DIRECTION_TOP:
					if (enemy.rect.y < WINDOW_HEIGHT * 0.5 - surEnemyEasy_1->h)
					{
						enemy.rect.y += ENEMY_MEDIUM_SPEED;
					}
					else
					{
						enemy.direction = rand() % 2 == 0 ? ENTRY_DIRECTION_LEFT : ENTRY_DIRECTION_RIGHT;
					}
					break;
				case ENTRY_DIRECTION_LEFT:
					if (enemy.rect.x < WINDOW_WIDTH - surEnemyEasy_1->w)
					{
						enemy.rect.x += ENEMY_MEDIUM_SPEED;
					}
					else
					{
						enemy.direction = ENTRY_DIRECTION_RIGHT;
					}
					break;
				case ENTRY_DIRECTION_RIGHT:
					if (enemy.rect.x > 0)
					{
						enemy.rect.x -= ENEMY_MEDIUM_SPEED;
					}
					else
					{
						enemy.direction = ENTRY_DIRECTION_LEFT;
					}
					break;
				}
				SDL_RenderCopy(rend, texEnemyMedium_1, NULL, &enemy.rect);
				break;
			case ENEMY_TYPE_BOSS:
				switch (enemy.direction)
				{
				case ENTRY_DIRECTION_TOP:
					if (enemy.rect.y < WINDOW_HEIGHT * 0.5 - surEnemyEasy_1->h)
					{
						enemy.rect.y += ENEMY_BOSS_SPEED;
					}
					else
					{
						enemy.direction = rand() % 2 == 0 ? ENTRY_DIRECTION_LEFT : ENTRY_DIRECTION_RIGHT;
					}
					break;
				case ENTRY_DIRECTION_LEFT:
					if (enemy.rect.x < WINDOW_WIDTH - surEnemyEasy_1->w)
					{
						enemy.rect.x += ENEMY_BOSS_SPEED;
					}
					else
					{
						enemy.direction = ENTRY_DIRECTION_RIGHT;
					}
					break;
				case ENTRY_DIRECTION_RIGHT:
					if (enemy.rect.x > 0)
					{
						enemy.rect.x -= ENEMY_BOSS_SPEED;
					}
					else
					{
						enemy.direction = ENTRY_DIRECTION_LEFT;
					}
					break;
				}
				SDL_RenderCopy(rend, texEnemyBoss_1, NULL, &enemy.rect);
				break;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_Window* window = SDL_CreateWindow("Deep Space Hero v1.0",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// 开始循环播放游戏背景音乐
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);    
	Mix_Music* backMusic = Mix_LoadMUS("./audio/back.mp3");
	Mix_PlayMusic(backMusic, -1);

	surHeroNormal_1 = IMG_Load("./images/me1.png");
	surHeroNormal_2 = IMG_Load("./images/me2.png");
	surEnemyEasy_1 = IMG_Load("./images/enemy1.png");
	surEnemyEasy_2 = IMG_Load("./images/enemy1_down1.png");
	surEnemyEasy_3 = IMG_Load("./images/enemy1_down2.png");
	surEnemyEasy_4 = IMG_Load("./images/enemy1_down3.png");
	surEnemyEasy_5 = IMG_Load("./images/enemy1_down4.png");
	surEnemyMedium_1 = IMG_Load("./images/enemy2.png");
	surEnemyMedium_2 = IMG_Load("./images/enemy2_down1.png");
	surEnemyMedium_3 = IMG_Load("./images/enemy2_down2.png");
	surEnemyMedium_4 = IMG_Load("./images/enemy2_down3.png");
	surEnemyMedium_5 = IMG_Load("./images/enemy2_down4.png");
	surEnemyBoss_1 = IMG_Load("./images/enemy3_n1.png");
	surEnemyBoss_2 = IMG_Load("./images/enemy3_down1.png");
	surEnemyBoss_3 = IMG_Load("./images/enemy3_down2.png");
	surEnemyBoss_4 = IMG_Load("./images/enemy3_down3.png");
	surEnemyBoss_5 = IMG_Load("./images/enemy3_down4.png");
	surEnemyBoss_6 = IMG_Load("./images/enemy3_down5.png");
	surEnemyBoss_7 = IMG_Load("./images/enemy3_down6.png");
	surBackground = IMG_Load("./images/background.png");
	surBulletHero = IMG_Load("./images/bullet2.png");
	surBulletEnemy = IMG_Load("./images/bullet1.png");

	texHeroNormal_1 = SDL_CreateTextureFromSurface(rend, surHeroNormal_1);
	texHeroNormal_2 = SDL_CreateTextureFromSurface(rend, surHeroNormal_2);
	texEnemyEasy_1 = SDL_CreateTextureFromSurface(rend, surEnemyEasy_1);
	texEnemyEasy_2 = SDL_CreateTextureFromSurface(rend, surEnemyEasy_2);
	texEnemyEasy_3 = SDL_CreateTextureFromSurface(rend, surEnemyEasy_3);
	texEnemyEasy_4 = SDL_CreateTextureFromSurface(rend, surEnemyEasy_4);
	texEnemyEasy_5 = SDL_CreateTextureFromSurface(rend, surEnemyEasy_5);
	texEnemyMedium_1 = SDL_CreateTextureFromSurface(rend, surEnemyMedium_1);
	texEnemyMedium_2 = SDL_CreateTextureFromSurface(rend, surEnemyMedium_2);
	texEnemyMedium_3 = SDL_CreateTextureFromSurface(rend, surEnemyMedium_3);
	texEnemyMedium_4 = SDL_CreateTextureFromSurface(rend, surEnemyMedium_4);
	texEnemyMedium_5 = SDL_CreateTextureFromSurface(rend, surEnemyMedium_5);
	texEnemyBoss_1 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_1);
	texEnemyBoss_2 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_2);
	texEnemyBoss_3 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_3);
	texEnemyBoss_4 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_4);
	texEnemyBoss_5 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_5);
	texEnemyBoss_6 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_6);
	texEnemyBoss_7 = SDL_CreateTextureFromSurface(rend, surEnemyBoss_7);
	texBackground = SDL_CreateTextureFromSurface(rend, surBackground);
	texBulletHero = SDL_CreateTextureFromSurface(rend, surBulletHero);
	texBulletEnemy = SDL_CreateTextureFromSurface(rend, surBulletEnemy);
	texCurrentHero = texHeroNormal_1;	// 将当前英雄战机的Texture初始化为状态1

	SDL_SetTextureBlendMode(texHeroNormal_1, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(texBackground, SDL_BLENDMODE_BLEND);

	SDL_SetTextureAlphaMod(texBackground, 75);
	
	cutYCoord = surBackground->h;

	rectHero.w = surHeroNormal_1->w;
	rectHero.h = surHeroNormal_1->h;
	rectHero.x = WINDOW_WIDTH / 2 - rectHero.w / 2;
	rectHero.y = WINDOW_HEIGHT * 0.8;

	SDL_Event event;
	bool bQuit = false;
	while (!bQuit)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				bQuit = true;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_UP:
					if (rectHero.y > WINDOW_HEIGHT * 0.1)
					{
						rectHero.y -= MOVE_SPEED;
					}
					break;
				case SDLK_DOWN:
					if (rectHero.y < WINDOW_HEIGHT - surHeroNormal_1->h)
					{
						rectHero.y += MOVE_SPEED;
					}
					break;
				case SDLK_LEFT:
					if (rectHero.x > 0)
					{
						rectHero.x -= MOVE_SPEED;
					}
					break;
				case SDLK_RIGHT:
					if (rectHero.x < WINDOW_WIDTH - surHeroNormal_1->w)
					{
						rectHero.x += MOVE_SPEED;
					}
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		if (timerHeroBullet >= HERO_BULLET_INTERVAL)
		{
			fireHeroBullets(rend);
			timerHeroBullet = 0;
		}
		
		timerHeroBullet++;

		updateHeroBullets(rend);

		// 通过裁剪背景图实现背景移动动画

		SDL_Rect rectCut;
		SDL_Rect rectPlace;

		rectCut.x = 0;
		rectCut.y = cutYCoord;
		rectCut.h = surBackground->h - cutYCoord;
		rectCut.w = surBackground->w;

		rectPlace.x = 0;
		rectPlace.y = 0;
		rectPlace.h = rectCut.h;
		rectPlace.w = WINDOW_WIDTH;

		SDL_RenderCopy(rend, texBackground, &rectCut, &rectPlace);

		rectPlace.x = 0;
		rectPlace.y = rectCut.h;
		rectPlace.h = WINDOW_HEIGHT - rectCut.h;
		rectPlace.w = WINDOW_WIDTH;

		rectCut.x = 0;
		rectCut.y = 0;
		rectCut.h = cutYCoord;
		rectCut.w = surBackground->w;

		SDL_RenderCopy(rend, texBackground, &rectCut, &rectPlace);

		// 通过切换飞机图片实现尾部喷气动画

		if (timerHeroImg >= HERO_ANIM_SPEED)
		{
			texCurrentHero = texCurrentHero == texHeroNormal_1 ? texHeroNormal_2 : texHeroNormal_1;
			timerHeroImg = 0;
		}

		timerHeroImg++;

		SDL_RenderCopy(rend, texCurrentHero, NULL, &rectHero);

		if (timerEnemyGenerate >= ENEMY_GENE_MIN_INTERVAL)
		{
			generateEnemy(rend);
			timerEnemyGenerate = 0;
		}

		timerEnemyGenerate++;

		updateEnmies(rend);

		fireEnemiesBullets(rend);
		updateEnemiesBullets(rend);

		// 刷新显示当前帧
		SDL_RenderPresent(rend);

		// 更新背景图片的裁剪坐标
		if (cutYCoord > 0)
		{
			cutYCoord -= BACKIMG_SPEED;
		}
		else
		{
			cutYCoord = surBackground->h;
		}

		SDL_Delay(10);
	}

	SDL_DestroyWindow(window);
	return 0;
}

