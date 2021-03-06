
#ifndef _CAMERA_H
#define _CAMERA_H

#include "GridDefines.h"

class ViewPoint;
class WorldObject;
class UpdateData;
class WorldPacket;

/// Camera - object-receiver. Receives broadcast packets from nearby worldobjects, object visibility changes and sends them to client
class DIAMOND_DLL_SPEC Camera
{
    friend class ViewPoint;
    public:

        Camera(Player* pl);
        virtual ~Camera();

        WorldObject* getBody() { return m_source;}
        Player* getOwner() { return &m_owner;}

        // set camera's view to any worldobject
        // Note: this worldobject must be in same map, in same phase with camera's owner(player)
        // client supports only unit and dynamic objects as farsight objects
        void SetView(WorldObject *obj, uint32 caused_by_aura_id = 0);

        // set view to camera's owner
        void ResetView();

        template<class T>
        void UpdateVisibilityOf(T * obj, UpdateData &d, std::set<WorldObject*>& vis);
        void UpdateVisibilityOf(WorldObject* obj);

        void ReceivePacket(WorldPacket *data);
        void UpdateVisibilityForOwner();    // updates visibility of worldobjects around viewpoint for camera's owner

    private:
        // called when viewpoint changes visibility state
        void Event_AddedToWorld();
        void Event_RemovedFromWorld();
        void Event_Moved();
        void Event_ViewPointVisibilityChanged();

        Player & m_owner;
        WorldObject *m_source;
        uint32 caused_by_aura;

        void UpdateForCurrentViewPoint();

    public:
        GridReference<Camera>& GetGridRef() { return m_gridRef; }
        bool isActiveObject() const { return false; }
    private:
        GridReference<Camera> m_gridRef;
};

/// Object-observer, notifies farsight object state to cameras that attached to it
class DIAMOND_DLL_SPEC ViewPoint
{
    friend class Camera;

    std::list<Camera*> m_cameras;
    std::list<Camera*>::iterator camera_iter;
    GridType * m_grid;
    
    void Attach(Camera* c) { m_cameras.push_back(c); }

    void Detach(Camera* c)
    {
        if (*camera_iter == c)     // detach called during the loop
            camera_iter = m_cameras.erase(camera_iter);
        else
            m_cameras.remove(c);
    }

    void CameraCall(void (Camera::*handler)(void))
    {
        for(camera_iter = m_cameras.begin(); camera_iter!=m_cameras.end(); ++camera_iter)
            ((*camera_iter)->*handler)();
    }

public:

    ViewPoint() : m_grid(0), camera_iter(m_cameras.end()) {}
    virtual ~ViewPoint();

    // these events are called when viewpoint changes visibility state
    void Event_AddedToMap(GridType *grid)
    {
        m_grid = grid;
        if(!m_cameras.empty())
            CameraCall(&Camera::Event_AddedToWorld);
    }

    void Event_RemovedFromMap()
    {
        m_grid = NULL;
        if(!m_cameras.empty())
            CameraCall(&Camera::Event_RemovedFromWorld);
    }

    void Event_GridChanged(GridType *grid)
    {
        m_grid = grid;
        if(!m_cameras.empty())
            CameraCall(&Camera::Event_Moved);
    }

    void Event_ViewPointVisibilityChanged()
    {
        if(!m_cameras.empty())
            CameraCall(&Camera::Event_ViewPointVisibilityChanged);
    }

    void Call_UpdateVisibilityForOwner()
    {
        if(!m_cameras.empty())
            CameraCall(&Camera::UpdateVisibilityForOwner);
    }
};



#endif