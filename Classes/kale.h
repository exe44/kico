/*
 *  kale.h
 *  kale
 *
 *  Created by exe on 8/16/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef KALE_KALE_H
#define KALE_KALE_H

#include <vector>

#include "input_mgr.h"
#include "scene_mgr.h"
#include "math_helper.h"

namespace ERI
{
	class CameraActor;
	class LightActor;
	class RenderToTexture;
	class SpriteActor;
	struct Texture;
}

class b2World;
class b2Body;

class Mirror;
class CollisionObj;
class TriangleMirror;
class KaleContactListener;
class LogoShower;
class BlackMask;
class MenuButton;
class Menu;
template<class> class Morpher;

enum LayerType
{
	LAYER_SCENE = 0,
	LAYER_MASK,
	LAYER_UI,
	LAYER_UI2,
	LAYER_MAX
};

class kaleApp : public ERI::Observer<ERI::SceneMgr::ResizeInfo>, public ERI::InputHandler
{
public:
	kaleApp();
	~kaleApp();
	
	void Init();
	void OnTerminate();
	void OnResignActive();
	void Release();
	
	void Update(float delta_time);
	
	virtual void Click(const ERI::InputEvent& event);
	virtual void MultiMove(const ERI::InputEvent* events, int num, bool is_start);
	virtual void Accelerate(const ERI::Vector3& g);
	virtual void Shake();
	
	virtual void OnNotified(ERI::SceneMgr::ResizeInfo& info);
	
	void SetIsAutoMode(bool is_auto_mode);
	
	void NotifyBlackMaskBetweenFadeInOut();
	
	inline static kaleApp& Ins()
	{
		if (!ins_ptr_)	ins_ptr_ = new kaleApp();
		return *ins_ptr_;
	}
	
	inline static void KillIns()
	{
		if (ins_ptr_)
		{
			delete ins_ptr_;
			ins_ptr_ = NULL;
		}
	}
	
	inline bool is_auto_mode() { return is_auto_mode_; }
	inline void set_is_sound_on(bool is_sound_on) { is_sound_on_ = is_sound_on; }
	inline bool is_sound_on() {return is_sound_on_; }
	
	static const int kBoundaryHalfSize;
	
private:
	void InitPhysics();
	void InitBoundary();
	
	void ResetCollisionObjs(bool is_first_time = false);
	void ClearCollisionObjs();
	
	void UpdateAuto(float delta_time);
	void UpdateWorldTransform(float delta_time);
	void UpdateAtmosphere(float delta_time);
	
	void LoadOption();
	void SaveOption();
	
	static kaleApp*			ins_ptr_;
	
	ERI::CameraActor*		cam_;
	ERI::CameraActor*		scene_cam_;
	
	ERI::LightActor*		light_;

	Mirror*					mirror_;
	
	ERI::SpriteActor*		atmosphere_mask_;
	const ERI::Texture*		atmosphere_texture_;
	float					atmosphere_u_;
	Morpher<float>*			atmosphere_v_blender_;
	
	ERI::Vector3			accelerator_g_;
	
	std::vector<CollisionObj*>	collision_objs_;
	
	b2World*				world_;
	std::vector<b2Body*>	boundary_bodys_;
	std::vector<b2Body*>	collision_bodys_;
	KaleContactListener*	contact_listener_;

	ERI::SpriteActor*		screen_dark_corner_mask_;

	LogoShower*				logo_shower_;
	BlackMask*				black_mask_;
	MenuButton*				menu_button_;
	Menu*					menu_;

	float					wait_button_remain_time_;
	float					auto_choose_g_remain_time_;
	float					auto_reset_remain_time_;
	
	bool					is_menu_mode_;
	bool					is_auto_mode_;
	bool					is_sound_on_;
};

#endif // KALE_KALE_H