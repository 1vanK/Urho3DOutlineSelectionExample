#pragma once


class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

public:
    Game(Context* context);

    virtual void Setup();
    virtual void Start();

protected:
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    float yaw_;
    float pitch_;
    Node* hoveredNode_ = nullptr;

private:
    void CreateScene();
    void SetupViewport();
    void MoveCamera(float timeStep);
    void SubscribeToEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void CreateUI();
    Node* Raycast();
};
