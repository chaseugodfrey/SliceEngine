/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        Singleton.h

 author:   Gideon Francis (100%)

 email:       g.francis@digipen.edu

 brief:      A singleton template class for ease of making singleton classes

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef SINGLETON_TEMPLATE_H
#define SINGLETON_TEMPLATE_H

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		static T instance;


		return &instance;
	}

protected:
	Singleton() {};
	virtual ~Singleton() = default;
};




#endif // SINGLETON_TEMPLATE_H
