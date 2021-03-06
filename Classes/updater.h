/*
 *  updater.h
 *  kale
 *
 *  Created by exe on 9/12/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef KALE_UPDATER_H
#define KALE_UPDATER_H

#include "math_helper.h"

class Updater
{
public:
	virtual ~Updater() {}
	virtual void Update(float delta_time) = 0;
};

class kaleApp;
template<class> class Morpher;

namespace ERI
{
	class SpriteActor;
	class TxtActor;
}

class LogoShower : public Updater
{
public:
	LogoShower();
	virtual ~LogoShower();
	
	virtual void Update(float delta_time);
	
	void Hide();
	void Show();
	
	inline bool is_finished() { return is_finished_; }
	
private:
	ERI::TxtActor*		logo_;
	
	Morpher<float>*		logo_fade_in_morpher_;
	Morpher<float>*		logo_fade_out_morpher_;
	
	float	wait_logo_remain_time_;
	float	logo_keep_remain_time_;
	
	bool	is_finished_;
};

class MenuButton : public Updater
{
public:
	MenuButton();
	virtual ~MenuButton();
	
	void RefreshScreenSize(float screen_width, float screen_height);
	
	virtual void Update(float delta_time);
	
	void FadeIn();
	void FadeOut();
	void FadeInOut();
	
	void Hide();
	void Show();
	
	bool IsHit(const ERI::Vector3& world_pos);
	
	void NotifyAtmosphereChange(const ERI::Color& atmosphere_color);
	
private:
	ERI::SpriteActor*	button_;

	Morpher<float>*		fade_in_morpher_;
	Morpher<float>*		fade_out_morpher_;
	
	float	keep_remain_time_;
};

class BlackMask : public Updater
{
public:
	BlackMask(kaleApp* app);
	virtual ~BlackMask();
	
	void SetSize(float width, float height);
	
	virtual void Update(float delta_time);
	
	void FadeIn(float in_time, float target_alpha = 1.0f);
	void FadeOut(float out_time);
	void FadeInOut(float in_time, float out_time);
	
	void Hide();
	void Show();
	
private:
	kaleApp*			app_ref_;
	
	ERI::SpriteActor*	mask_;
	
	Morpher<float>*		fade_in_morpher_;
	Morpher<float>*		fade_out_morpher_;
};

class Menu : public Updater
{
public:
	Menu(kaleApp* app);
	virtual ~Menu();
	
	virtual void Update(float delta_time);
	
	void FadeIn();
	void FadeOut();
	
	void Hide();
	void Show();
	
	void Click(const ERI::Vector3& world_pos);
	
private:
	void SetupAutoMode();
	
	static const float	auto_pic_change_time_;
	
	kaleApp*			app_ref_;
	
	ERI::SpriteActor*	sound_;
	ERI::SpriteActor*	auto_;
	
	int					auto_pic_idx_;
	float				auto_pic_change_remain_time_;
	
	ERI::TxtActor*		txt_;
	ERI::TxtActor*		txt2_;
	
	Morpher<float>*		fade_in_morpher_;
	Morpher<float>*		fade_out_morpher_;
};

#endif // KALE_UPDATER_H