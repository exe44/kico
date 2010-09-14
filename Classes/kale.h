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

class kaleApp : public ERI::InputHandler
{
public:
	kaleApp();
	~kaleApp();
	
	void Init();
	void OnTerminate();
	void Release();
	
	void Update(float delta_time);
	
	virtual void Click(int screen_x, int screen_y);
	virtual void MultiMove(const ERI::Vector2* moves, int num, bool is_start);
	virtual void Accelerate(float g_x, float g_y, float g_z);
	virtual void Shake();
	
	inline static kaleApp& Ins()
	{
		if (!ins_ptr_)	ins_ptr_ = new kaleApp();
		return *ins_ptr_;
	}
	inline static kaleApp* InsPtr()
	{
		if (!ins_ptr_)	ins_ptr_ = new kaleApp();
		return ins_ptr_;
	}
	
	inline void set_is_auto_mode(bool is_auto_mode) { is_auto_mode_ = is_auto_mode; auto_choose_remain_time_ = 0.0f; }
	inline bool is_auto_mode() { return is_auto_mode_; }
	inline void set_is_sound_on(bool is_sound_on) { is_sound_on_ = is_sound_on; }
	inline bool is_sound_on() {return is_sound_on_; }
	
	inline int mask_layer() { return mask_layer_; }
	inline int ui_layer() { return ui_layer_; }
	inline int ui_layer2() { return ui_layer2_; }
	
	static const int kBoundaryHalfSize;
	
private:
	void InitPhysics();
	void InitBoundary();
	
	void ResetCollisionObjs();
	void ClearCollisionObjs();
	
	void UpdateAuto(float delta_time);
	void UpdateWorldTransform(float delta_time);
	void UpdateAtmosphere(float delta_time);
	
	static kaleApp*			ins_ptr_;
	
	ERI::CameraActor*		cam_;
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
	
	bool					is_menu_mode_;
	bool					is_auto_mode_;
	bool					is_sound_on_;
	
	float					auto_choose_remain_time_;
	
	int						mask_layer_, ui_layer_, ui_layer2_;
};

#endif // KALE_KALE_H