/*
 *  helper.h
 *  kale
 *
 *  Created by exe on 8/18/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef KALE_HELPER_H
#define KALE_HELPER_H

#pragma mark Morpher

template<class T>
class Morpher
{
public:
	Morpher(T speed, T tolerance) :
		current_value_(0),
		start_value_(0),
		target_value_(0),
		speed_(speed),
		tolerance_(tolerance)
	{
		ASSERT(speed_ > 0);
		ASSERT(tolerance_ >= 0);
	}
	
	~Morpher()
	{
	}
	
	void SetSpeed(T speed)
	{
		speed_ = speed;
	}
	
	void SetCurrent(T current_value)
	{
		current_value_ = current_value;
		start_value_ = current_value;
	}
	
	void SetTarget(T target_value)
	{
		target_value_ = target_value;
	}
	
	void Update(float delta_time)
	{
		T diff = target_value_ - current_value_;
		
		if (diff > tolerance_)
		{
			current_value_ += speed_ * delta_time;
			if ((tolerance_ == 0) && (current_value_ > target_value_)) current_value_ = target_value_;
		}
		else if (diff < -tolerance_)
		{
			current_value_ -= speed_ * delta_time;
			if ((tolerance_ == 0) && (current_value_ < target_value_)) current_value_ = target_value_;
		}
	}
	
	float GetPercent()
	{
		return static_cast<float>(current_value_ - start_value_) / static_cast<float>(target_value_ - start_value_);
	}
	
	void Finish()
	{
		target_value_ = current_value_;
	}
	
	inline bool IsFinished()
	{
		T diff = target_value_ - current_value_;

		if (tolerance_ == 0)
			return (diff == 0);
		
		return (diff > -tolerance_ && diff < tolerance_);
	}
	
	inline T current_value() { return current_value_; }
	
private:
	T	current_value_, start_value_, target_value_, speed_, tolerance_;
};

#endif // KALE_HELPER_H