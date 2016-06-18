#include "GameManager.h"

GameManager* GameManager::instance = nullptr;
GameManager* GameManager::getInstance()
{
	if (instance == nullptr)
	{
		instance = new GameManager();
		PokerArrayUtil::initFunction();
	}
	return instance;
}

void GameManager::increaseCount(Ref *ref)
{
	numberOfLoadRes++;
}
void GameManager::initCard(Ref * ref)
{
	int type = 0;
	auto texture = Director::getInstance()->getTextureCache()->getTextureForKey("puke_whole.png");
	auto cardWidth = texture->getContentSize().width / 9;
	auto cardHeight = texture->getContentSize().height / 6;
	for (int i = 0; i < 54; i++)
	{
		if (i % 13 == 0)
		{
			type++;
		}
		int row = i / 9;
		int col = i - 9 * row;

		auto originX = texture->getContentSize().width - (col + 1) * cardWidth;
		auto originY = texture->getContentSize().height - (6 - row) * cardHeight;
		Rect rect(originX, originY, cardWidth, cardHeight);
		auto card = PokerCard::create(SpriteFrame::createWithTexture(texture, rect));
		card->setType(type);
		card->setValueInAll(i);
		int value = i % 13 + 1;
		//set the value of card beyond three with its number
		if (value > 2)
		{
			card->setValueInType(value);
		}
		//set the value of card 1 and card 2 as 14 and 15 respectively
		else
		{
			card->setValueInType(value + 13);
		}
		//set the value of small queen and big queen as 16 and 16 respectively
		if (i >= 52)
		{
			card->setValueInType(i % 13 + 16);
		}
		rawCardArray.pushBack(card);
	}
	log("Total Card : %d", rawCardArray.size());
	increaseCount(ref);
}

void GameManager::initNumbers(Ref * ref)
{
	auto texture = Director::getInstance()->getTextureCache()->getTextureForKey("poker_number.png");
	auto width = texture->getContentSize().width / 10;
	auto height = texture->getContentSize().height;
	for (int i = 0; i < 10; i++)
	{
		auto sprite = Sprite::createWithTexture(texture, Rect(i * width, 0, width, height));
		numberArray.pushBack(sprite);
	}
	increaseCount(ref);
}

GameManager::GameManager()
{
	//Ԥ������Դ
	//instance = nullptr;
	numberOfTotalRes = 7;
	numberOfLoadRes = 0;
	srand(time(nullptr));

	Director::getInstance()->getTextureCache()->addImageAsync("logo.png", CC_CALLBACK_1(GameManager::increaseCount, this));
	Director::getInstance()->getTextureCache()->addImageAsync("logo_dizhu.png", CC_CALLBACK_1(GameManager::increaseCount, this));
	Director::getInstance()->getTextureCache()->addImageAsync("logo_nongmin.png", CC_CALLBACK_1(GameManager::increaseCount, this));
	Director::getInstance()->getTextureCache()->addImageAsync("logo_unknown.png", CC_CALLBACK_1(GameManager::increaseCount, this));
	Director::getInstance()->getTextureCache()->addImageAsync("poke_back_header.png", CC_CALLBACK_1(GameManager::increaseCount, this));
	Director::getInstance()->getTextureCache()->addImageAsync("poker_number.png", CC_CALLBACK_1(GameManager::initNumbers, this));
	Director::getInstance()->getTextureCache()->addImageAsync("puke_whole.png", CC_CALLBACK_1(GameManager::initCard, this));
	
}