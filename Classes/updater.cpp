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

#include "kale.h"
#include "helper.h"

using namespace ERI;

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
	logo_->AddToScene(app_ref_->ui_layer());
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

MenuButton::MenuButton(kaleApp* app) :
	app_ref_(app),
	keep_remain_time_(0.0f)
{
	button_ = new SpriteActor(32, 32);
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

Menu::Menu(kaleApp* app) :
	app_ref_(app)
{
	txt_ = new TxtActor("KICO by exe", "futura", 16, true);
	txt_->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	txt_->AddToScene(app_ref_->ui_layer2());
	txt_->SetPos(Vector3(0, 10, 10));
	txt_->SetColor(Color(1, 1, 1, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(txt_);
	
	txt2_ = new TxtActor("(C) 2009 all rights reserved.", "futura", 14, true);
	txt2_->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	txt2_->AddToScene(app_ref_->ui_layer2());
	txt2_->SetPos(Vector3(0, -10, 10));
	txt2_->SetColor(Color(0.5f, 0.5f, 0.5f, 0));
	Root::Ins().scene_mgr()->current_cam()->AddChild(txt2_);

	fade_in_morpher_ = new Morpher<float>(4.0f, 0.0f);
	fade_out_morpher_ = new Morpher<float>(4.0f, 0.0f);
}

Menu::~Menu()
{
	delete fade_out_morpher_;
	delete fade_in_morpher_;
	delete txt2_;
	delete txt_;
}

void Menu::Update(float delta_time)
{
	if (!fade_in_morpher_->IsFinished())
	{
		fade_in_morpher_->Update(delta_time);
		
		Color color = txt_->GetColor();
		color.a = fade_in_morpher_->current_value();
		txt_->SetColor(color);

		color = txt2_->GetColor();
		color.a = fade_in_morpher_->current_value();
		txt2_->SetColor(color);
		
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
}

void Menu::Show()
{
	if (txt_->GetColor().a > 0.0f)
	{
		txt_->set_visible(true);
		txt2_->set_visible(true);
	}
}

