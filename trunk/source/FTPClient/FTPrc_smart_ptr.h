
/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster based on Copyright (c) 2004 Thomas Oswald
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 *
 * for WiiXplorer 2009
 ***************************************************************************/


#ifndef INC_RC_SMART_PTR_H
#define INC_RC_SMART_PTR_H

namespace nsSP
{
// base class for reference-counted objects
class RCObject
{
public:                                
   void AddReference() { ++m_iRefCount; }
   void RemoveReference()
   {
      if (--m_iRefCount == 0)
         delete this;
   }

   void MarkUnshareable() { m_fShareable = false; }
   bool IsShareable() const { return m_fShareable; }
   bool IsShared() const { return m_iRefCount > 1; }

protected:
   RCObject() : m_iRefCount(0), m_fShareable(true) {}
   RCObject(const RCObject& /*rhs*/) : m_iRefCount(0), m_fShareable(true) {}
   RCObject& operator=(const RCObject& /*rhs*/) { return *this; }

   virtual ~RCObject() {}

private:
   int  m_iRefCount;
   bool m_fShareable;
};

/******************************************************************************
*                 Template Class RCPtr 
******************************************************************************/
// template class for smart pointers-to-T objects; T must support the RCObject interface
template<class T>
class RCPtr
{
public:
   RCPtr(T* realPtr = 0) : m_Pointee(realPtr) { Init(); }
   RCPtr(const RCPtr& rhs) : m_Pointee(rhs.m_Pointee) { Init(); }
   ~RCPtr() { if (m_Pointee) m_Pointee->RemoveReference(); }
   RCPtr& operator=(const RCPtr& rhs)
   {
      if (m_Pointee != rhs.m_Pointee)
      {
         T* pOldPointee = m_Pointee;
         m_Pointee = rhs.m_Pointee;
         Init(); 

         if (pOldPointee)
            pOldPointee->RemoveReference();                
      }

      return *this;
   }
   T* operator->() const { return m_Pointee; };
   T& operator*() const { return *m_Pointee; };

   bool IsNull() const  { return m_Pointee==NULL; }
   bool IsValid() const { return m_Pointee!=NULL; }

private:
   T* m_Pointee;

   void Init()
   {
      if (m_Pointee == 0)
         return;

      if (m_Pointee->IsShareable() == false)
         m_Pointee = new T(*m_Pointee);
   
      m_Pointee->AddReference();
   }
};


template<class T>
class RCIPtr
{
public:
   RCIPtr(T* realPtr = 0) :
      m_pCounter(new CountHolder)
   { 
      m_pCounter->m_Pointee = realPtr;
      Init();
   }

   RCIPtr(const RCIPtr& rhs) : m_pCounter(rhs.m_pCounter) { Init(); }
   ~RCIPtr() { m_pCounter->RemoveReference(); }
   RCIPtr& operator=(const RCIPtr& rhs)
   {
      if (m_pCounter != rhs.m_pCounter)
      {
         m_pCounter->RemoveReference();     
         m_pCounter = rhs.m_pCounter;
         Init();
      }
      return *this;
   }

   T* operator->() const { return m_pCounter->m_Pointee; }
   T& operator*() const { return *(m_pCounter->m_Pointee); }

   RCObject& GetRCObject() { return *m_pCounter; }

   bool IsNull() const  { return m_pCounter==NULL?true:m_pCounter->m_Pointee==NULL; }
   bool IsValid() const { return m_pCounter==NULL?false:m_pCounter->m_Pointee!=NULL; }

private:
   struct CountHolder: public RCObject
   {
      ~CountHolder()
      { 
         delete m_Pointee;
      }
      T* m_Pointee;
   };

   CountHolder* m_pCounter;
   void Init()
   {
      if (m_pCounter->IsShareable() == false)
      {
         T* pOldValue = m_pCounter->m_Pointee;
         m_pCounter = new CountHolder;
         m_pCounter->m_Pointee = new T(*pOldValue);
      } 
      m_pCounter->AddReference();
   }
};

};
#endif // INC_RC_SMART_PTR_H
