/*
 *  updater.cpp
 *  kale
 *
 *  Created by exe on 9/12/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "updater.h"

#include "root.h"
#include "scene_mgr.h"
#include "scene_actor.h"
#include "audio_manager.h"

#include "kale.h"
#include "helper.h"

using namespace ERI;

#pragma mark LogoShower

static const float kWaitLogoTime = 3.0f;
static const float kLogoKeepTime = 2.0f;

LogoShower::LogoShower(kaleApp* app) :
	app_ref_(app),
	wait_logo_remain_time_(kWaitLogoTime),
	logo_keep_remain_time_(kLogoKeepTime),
	is_finished_(false)
{
	logo_ = new TxtActor("k i c o", "futura", 26);
	logo_->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	logo_->AddToScene(app_ref_->ui_layer2());
	logo_->SetPos(Vector3(-120, 40, 5));
	logo_->SetColor(Color(1, 1, 1, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(logo_);
	
	logo_fade_in_morpher_ = new Morpher<float>(0.75f, 0.0f);
	logo_fade_in_morpher_->SetCurrent(0.0f);
	logo_fade_in_morpher_->SetTarget(1.0f);
	
	logo_fade_out_morpher_ = new Morpher<float>(0.5f, 0.0f);
	logo_fade_out_morpher_->SetCurrent(1.0f);
	logo_fade_out_morpher_->SetTarget(0.0f);
}

LogoShower::~LogoShower()
{
	delete logo_fade_out_morpher_;
	delete logo_fade_in_morpher_;
	delete logo_;
}

void LogoShower::Update(float delta_time)
{
	if (wait_logo_remain_time_ > 0.0f)
	{
		wait_logo_remain_time_ -= delta_time;
		
		return;
	}
	
	if (!logo_fade_in_morpher_->IsFinished())
	{
		logo_fade_in_morpher_->Update(delta_time);
		Color color = logo_->GetColor();
		color.a = logo_fade_in_morpher_->current_value();
		logo_->SetColor(color);

		return;
	}
	
	if (logo_keep_remain_time_ > 0.0f)
	{
		logo_keep_remain_time_ -= delta_time;
		
		return;
	}
	
	if (!logo_fade_out_morpher_->IsFinished())
	{
		logo_fade_out_morpher_->Update(delta_time);
		Color color = logo_->GetColor();
		color.a = logo_fade_out_morpher_->current_value();
		logo_->SetColor(color);
		
		if (logo_fade_out_morpher_->IsFinished())
		{
			is_finished_ = true;
		}
		
		return;
	}
}

void LogoShower::Hide()
{
	logo_->set_visible(false);
}

void LogoShower::Show()
{
	if (logo_->GetColor().a > 0.0f) logo_->set_visible(true);
}

#pragma mark MenuButton

MenuButton::MenuButton(kaleApp* app) :
	app_ref_(app),
	keep_remain_time_(0.0f)
{
	button_ = new SpriteActor(32, 32);
	button_->set_area_border(16);
	button_->SetMaterial("media/menu3.png", FILTER_LINEAR, FILTER_LINEAR);
	button_->BlendAdd();
	button_->SetPos(Vector3(115, -200, 4));
	button_->SetRotate(5);
	button_->AddToScene(app_ref_->ui_layer2());
	button_->SetColor(Color(0.5f, 0.5f, 0.5f, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(button_);
	
	fade_in_morpher_ = new Morpher<float>(1.0f, 0.0f);
	fade_out_morpher_ = new Morpher<float>(1.0f, 0.0f);
}

MenuButton::~MenuButton()
{
	delete fade_out_morpher_;
	delete fade_in_morpher_;
	delete button_;
}

void MenuButton::Update(float delta_time)
{
	if (!fade_in_morpher_->IsFinished())
	{
		fade_in_morpher_->Update(delta_time);
		Color color = button_->GetColor();
		color.a = fade_in_morpher_->current_value();
		button_->SetColor(color);
		
		return;
	}
	
	if (keep_remain_time_ > 0.0f)
	{
		keep_remain_time_ -= delta_time;
		
		return;
	}
	
	if (!fade_out_morpher_->IsFinished())
	{
		fade_out_morpher_->Update(delta_time);
		Color color = button_->GetColor();
		color.a = fade_out_morpher_->current_value();
		button_->SetColor(color);
		
		return;
	}
}

void MenuButton::FadeIn()
{
	float current = button_->GetColor().a;
	
	fade_in_morpher_->SetCurrent(current);
	fade_in_morpher_->SetTarget(1.0f);
	fade_in_morpher_->SetSpeed(4.0f);

	if (keep_remain_time_ > 0.0f)
		keep_remain_time_ = 0.0f;
	
	fade_out_morpher_->Finish();
}

void MenuButton::FadeOut()
{
	float current = button_->GetColor().a;
	
	fade_out_morpher_->SetCurrent(current);
	fade_out_morpher_->SetTarget(0.0f);
	fade_out_morpher_->SetSpeed(4.0f);
	
	if (keep_remain_time_ > 0.0f)
		keep_remain_time_ = 0.0f;
	
	fade_in_morpher_->Finish();
}

void MenuButton::FadeInOut()
{
	float current = button_->GetColor().a;
	
	fade_in_morpher_->SetCurrent(current);
	fade_in_morpher_->SetTarget(1.0f);
	fade_in_morpher_->SetSpeed(1.0f);
	
	keep_remain_time_ = 1.0f;
	
	fade_out_morpher_->SetCurrent(1.0f);
	fade_out_morpher_->SetTarget(0.0f);
	fade_out_morpher_->SetSpeed(0.33f);
}

void MenuButton::Hide()
{
	button_->set_visible(false);
}

void MenuButton::Show()
{
	if (button_->GetColor().a > 0.0f) button_->set_visible(true);
}

bool MenuButton::IsHit(const ERI::Vector3& world_pos)
{
	return button_->IsHit(world_pos);
}

void MenuButton::NotifyAtmosphereChange(const ERI::Color& atmosphere_color)
{
	Color color(0.5f, 0.5f, 0.5f);
	color += atmosphere_color * 0.5f;
	color.a = button_->GetColor().a;
	button_->SetColor(color);
}

#pragma mark BlackMask

BlackMask::BlackMask(kaleApp* app) :
	app_ref_(app)
{
	mask_ = new SpriteActor(320, 480);
	mask_->AddToScene(app_ref_->ui_layer());
	mask_->SetPos(Vector3(0, 0, 5));
	mask_->SetColor(Color(0, 0, 0, 1));
	Root::Ins().scene_mgr()->current_cam()->AddChild(mask_);
	
	fade_in_morpher_ = new Morpher<float>(1.0f, 0.0f);
	fade_out_morpher_ = new Morpher<float>(1.0f, 0.0f);
}

BlackMask::~BlackMask()
{
	delete fade_out_morpher_;
	delete fade_in_morpher_;
	delete mask_;
}

void BlackMask::Update(float delta_time)
{
	if (!fade_in_morpher_->IsFinished())
	{
		fade_in_morpher_->Update(delta_time);
		Color color = mask_->GetColor();
		color.a = fade_in_morpher_->current_value();
		mask_->SetColor(color);
		
		if (fade_in_morpher_->IsFinished() && !fade_out_morpher_->IsFinished())
		{
			app_ref_->NotifyBlackMaskBetweenFadeInOut();
		}
		
		return;
	}
	
	if (!fade_out_morpher_->IsFinished())
	{
		fade_out_morpher_->Update(delta_time);
		Color color = mask_->GetColor();
		color.a = fade_out_morpher_->current_value();
		mask_->SetColor(color);
		
		return;
	}
}

void BlackMask::FadeIn(float in_time, float target_alpha /*= 1.0f*/)
{
	float current = mask_->GetColor().a;
	
	fade_in_morpher_->SetCurrent(current);
	fade_in_morpher_->SetTarget(target_alpha);
	
	float speed = (target_alpha - current) / in_time;
	if (speed < 0) speed *= -1;
	
	fade_in_morpher_->SetSpeed(speed);

	fade_out_morpher_->Finish();
}

void BlackMask::FadeOut(float out_time)
{
	float current = mask_->GetColor().a;
	
	fade_out_morpher_->SetCurrent(current);
	fade_out_morpher_->SetTarget(0.0f);
	
	float speed = (0.0f - current) / out_time;
	if (speed < 0) speed *= -1;
	
	fade_out_morpher_->SetSpeed(speed);
	
	fade_in_morpher_->Finish();
}

void BlackMask::FadeInOut(float in_time, float out_time)
{
	float current = mask_->GetColor().a;
	float speed;
	
	fade_in_morpher_->SetCurrent(current);
	fade_in_morpher_->SetTarget(1.0f);
	
	speed = (1.0f - current) / in_time;
	if (speed < 0) speed *= -1;
	
	fade_in_morpher_->SetSpeed(speed);
	
	fade_out_morpher_->SetCurrent(1.0f);
	fade_out_morpher_->SetTarget(0.0f);
	
	speed = 1.0f / out_time;
	
	fade_out_morpher_->SetSpeed(speed);
}

void BlackMask::Hide()
{
	mask_->set_visible(false);
}

void BlackMask::Show()
{
	if (mask_->GetColor().a > 0.0f) mask_->set_visible(true);
}

#pragma mark Menu

const float	Menu::auto_pic_change_time_ = 0.25f;

Menu::Menu(kaleApp* app) :
	app_ref_(app)
{
	txt_ = new TxtActor("KICO by exe", "futura", 16, true);
	txt_->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	txt_->AddToScene(app_ref_->ui_layer2());
	txt_->SetPos(Vector3(0, -60, 10));
	txt_->SetColor(Color(1, 1, 1, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(txt_);
	
	txt2_ = new TxtActor("(C) 2010 Wallmud & Exe", "futura", 14, true);
	txt2_->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	txt2_->AddToScene(app_ref_->ui_layer2());
	txt2_->SetPos(Vector3(0, -85, 10));
	txt2_->SetColor(Color(0.5f, 0.5f, 0.5f, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(txt2_);
	
	txt3_ = new TxtActor("All rights reserved.", "futura", 14, true);
	txt3_->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	txt3_->AddToScene(app_ref_->ui_layer2());
	txt3_->SetPos(Vector3(0, -100, 10));
	txt3_->SetColor(Color(0.5f, 0.5f, 0.5f, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(txt3_);
	
	sound_ = new SpriteActor(32, 32);
	sound_->set_area_border(16);
	sound_->SetMaterial(app_ref_->is_sound_on() ? "media/sound.png" : "media/sound_off.png", FILTER_LINEAR, FILTER_LINEAR);
	sound_->BlendAdd();
	sound_->AddToScene(app_ref_->ui_layer2());
	sound_->SetPos(Vector3(0, 130, 10));
	sound_->SetColor(Color(1, 1, 1, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(sound_);

	auto_ = new SpriteActor(32, 32);
	auto_->set_area_border(16);
	auto_->BlendAdd();
	auto_->AddToScene(app_ref_->ui_layer2());
	auto_->SetPos(Vector3(0, 50, 10));
	auto_->SetColor(Color(1, 1, 1, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(auto_);
	
	SetupAutoMode();

	fade_in_morpher_ = new Morpher<float>(4.0f, 0.0f);
	fade_out_morpher_ = new Morpher<float>(4.0f, 0.0f);
}

Menu::~Menu()
{
	delete fade_out_morpher_;
	delete fade_in_morpher_;
	delete auto_;
	delete sound_;
	delete txt3_;
	delete txt2_;
	delete txt_;
}

void Menu::Update(float delta_time)
{
	if (app_ref_->is_auto_mode())
	{
		auto_pic_change_remain_time_ -= delta_time;
		if (auto_pic_change_remain_time_ <= 0.0f)
		{
			++auto_pic_idx_;
			if (auto_pic_idx_ > 1) auto_pic_idx_ = 0;
			
			auto_->SetMaterial(auto_pic_idx_ ? "media/auto4.png" : "media/auto5.png", FILTER_LINEAR, FILTER_LINEAR);
			
			auto_pic_change_remain_time_ = auto_pic_change_time_;
		}
	}
	
	if (!fade_in_morpher_->IsFinished())
	{
		fade_in_morpher_->Update(delta_time);
		
		Color color = txt_->GetColor();
		color.a = fade_in_morpher_->current_value();
		txt_->SetColor(color);

		color = txt2_->GetColor();
		color.a = fade_in_morpher_->current_value();
		txt2_->SetColor(color);

		color = txt3_->GetColor();
		color.a = fade_in_morpher_->current_value();
		txt3_->SetColor(color);
		
		color = sound_->GetColor();
		color.a = fade_in_morpher_->current_value();
		sound_->SetColor(color);
		
		color = auto_->GetColor();
		color.a = fade_in_morpher_->current_value();
		auto_->SetColor(color);
		
		return;
	}
	
	if (!fade_out_morpher_->IsFinished())
	{
		fade_out_morpher_->Update(delta_time);
		
		Color color = txt_->GetColor();
		color.a = fade_out_morpher_->current_value();
		txt_->SetColor(color);
		
		color = txt2_->GetColor();
		color.a = fade_out_morpher_->current_value();
		txt2_->SetColor(color);
		
		color = txt3_->GetColor();
		color.a = fade_out_morpher_->current_value();
		txt3_->SetColor(color);
		
		color = sound_->GetColor();
		color.a = fade_out_morpher_->current_value();
		sound_->SetColor(color);
		
		color = auto_->GetColor();
		color.a = fade_out_morpher_->current_value();
		auto_->SetColor(color);
		
		return;
	}
}

void Menu::FadeIn()
{
	float current = txt_->GetColor().a;
	
	fade_in_morpher_->SetCurrent(current);
	fade_in_morpher_->SetTarget(1.0f);

	fade_out_morpher_->Finish();
}

void Menu::FadeOut()
{
	float current = txt_->GetColor().a;
	
	fade_out_morpher_->SetCurrent(current);
	fade_out_morpher_->SetTarget(0.0f);
	
	fade_in_morpher_->Finish();
}

void Menu::Hide()
{
	txt_->set_visible(false);
	txt2_->set_visible(false);
	txt3_->set_visible(false);
	sound_->set_visible(false);
	auto_->set_visible(false);
}

void Menu::Show()
{
	if (txt_->GetColor().a > 0.0f)
	{
		txt_->set_visible(true);
		txt2_->set_visible(true);
		txt3_->set_visible(true);
		sound_->set_visible(true);
		auto_->set_visible(true);
	}
}

void Menu::Click(const ERI::Vector3& world_pos)
{
	if (sound_->IsHit(world_pos))
	{
		app_ref_->set_is_sound_on(!app_ref_->is_sound_on());
		sound_->SetMaterial(app_ref_->is_sound_on() ? "media/sound.png" : "media/sound_off.png", FILTER_LINEAR, FILTER_LINEAR);
	}
	else if (auto_->IsHit(world_pos))
	{
		app_ref_->SetIsAutoMode(!app_ref_->is_auto_mode());
		
		SetupAutoMode();
	}
}

void Menu::SetupAutoMode()
{
	auto_->SetMaterial(app_ref_->is_auto_mode() ? "media/auto4.png" : "media/auto_off3.png", FILTER_LINEAR, FILTER_LINEAR);
	
	if (app_ref_->is_auto_mode())
	{
		auto_pic_idx_ = 0;
		auto_pic_change_remain_time_ = auto_pic_change_time_;
	}
}
