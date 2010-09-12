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

class CollisionObj;
class TriangleMirror;
template<class> class Morpher;
class b2World;
class b2Body;
class KaleContactListener;
class LogoShower;
class MenuButton;
class BlackMask;
class Menu;

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
	
	inline int mask_layer() { return mask_layer_; }
	inline int ui_layer() { return ui_layer_; }
	inline int ui_layer2() { return ui_layer2_; }
	
private:
	void InitPhysics();
	void InitBoundary();
	void ResetCollisionObjs();
	void ClearCollisionObjs();
	void UpdateWorldTransform(float delta_time);
	void UpdateAtmosphere(float delta_time);
	
	static kaleApp*		ins_ptr_;
	
	ERI::CameraActor*	cam_;
	ERI::LightActor*	light_;
	
	ERI::CameraActor*		mirror_cam_;
	ERI::RenderToTexture*	mirror_texture_;
	TriangleMirror*			mirror_;
	
	ERI::SpriteActor*	mirror_dark_corner_mask_;
	ERI::SpriteActor*	screen_dark_corner_mask_;
	ERI::SpriteActor*	atmosphere_mask_;
	
	ERI::Vector3		accelerator_g_;
	
	std::vector<CollisionObj*>	collision_objs_;
	
	const ERI::Texture*	atmosphere_texture_;
	float				atmosphere_u_;
	Morpher<float>*		atmosphere_v_blender_;
	
	b2World*				world_;
	std::vector<b2Body*>	boundary_bodys_;
	std::vector<b2Body*>	collision_bodys_;
	KaleContactListener*	contact_listener_;
	
	int		mask_layer_, ui_layer_, ui_layer2_;
	
	LogoShower*		logo_shower_;
	MenuButton*		menu_button_;
	BlackMask*		black_mask_;
	Menu*			menu_;
	
	bool	is_menu_mode_;
};

#endif // KALE_KALE_H