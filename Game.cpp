#include "Urho3DAll.h"
#include "Game.h"


URHO3D_DEFINE_APPLICATION_MAIN(Game)


Game::Game(Context* context) :
    Application(context),
    yaw_(0.0f),
    pitch_(0.0f)
{
}


void Game::Setup()
{
    engineParameters_["WindowTitle"] = GetTypeName();
    //engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
    engineParameters_["FullScreen"] = false;
    //engineParameters_["Headless"] = false;
    engineParameters_["WindowWidth"] = 800;
    engineParameters_["WindowHeight"] = 600;
    engineParameters_["ResourcePaths"] = "Data;CoreData;MyData";
}


void Game::Start()
{
    CreateScene();
    SetupViewport();
    CreateUI();
    SubscribeToEvents();

    GetSubsystem<Input>()->SetMouseVisible(true);
}


void Game::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    auto cache = GetSubsystem<ResourceCache>();
    renderer->SetDefaultRenderPath(cache->GetResource<XMLFile>("RenderPaths/MyForward.xml"));
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
    viewport->GetRenderPath()->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
}


void Game::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();

    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(100.0f, 1.0f, 100.0f));
    StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetColor(Color(0.5f, 0.5f, 0.5f));
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.5f, 0.5f, 0.5f));
    zone->SetFogColor(Color(0.4f, 0.5f, 0.8f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    for (int i = 0; i < 200; ++i)
    {
        auto node = scene_->CreateChild();
        node->SetPosition(Vector3(Random(90.0f) - 45.0f, 0.0f, Random(90.0f) - 45.0f));
        node->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        node->SetScale(0.5f + Random(2.0f));
        auto staticModel = node->CreateComponent<StaticModel>();
        staticModel->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
        auto material = cache->GetResource<Material>("Materials/MushroomOutline.xml")->Clone();
        staticModel->SetMaterial(material);
        staticModel->SetCastShadows(true);
        node->AddTag("Selectable");
    }

    for (int i = 0; i < 200; ++i)
    {
        auto node = scene_->CreateChild();
        node->SetPosition(Vector3(Random(90.0f) - 45.0f, 1.0f, Random(90.0f) - 45.0f));
        node->SetScale(2);
        node->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        auto staticModel = node->CreateComponent<StaticModel>();
        staticModel->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        staticModel->SetCastShadows(true);
    }

    cameraNode_ = scene_->CreateChild("Camera");
    cameraNode_->CreateComponent<Camera>();
    cameraNode_->SetPosition(Vector3(60.0f, 20.0f, -60.0f));
    cameraNode_->LookAt(Vector3::ZERO);
    yaw_ = cameraNode_->GetRotation().YawAngle();
    pitch_ = cameraNode_->GetRotation().PitchAngle();
}


Node* Game::Raycast()
{
    auto pos = GetSubsystem<Input>()->GetMousePosition();
    auto graphics = GetSubsystem<Graphics>();
    auto camera = cameraNode_->GetComponent<Camera>();
    auto x = (float)pos.x_ / graphics->GetWidth();
    auto y = (float)pos.y_ / graphics->GetHeight();
    auto cameraRay = camera->GetScreenRay(x, y);
    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000, DRAWABLE_GEOMETRY);
    //RayOctreeQuery query(results, cameraRay, RAY_OBB, 1000, DRAWABLE_GEOMETRY);
    scene_->GetComponent<Octree>()->RaycastSingle(query);
    if (results.Size() == 0)
        return nullptr;
    auto node = results[0].node_;
    if (node->HasTag("Selectable"))
        return node;
    return nullptr;
}


void Game::MoveCamera(float timeStep)
{
    Input* input = GetSubsystem<Input>();

    const float MOVE_SPEED = 20.0f;
    const float MOUSE_SENSITIVITY = 0.1f;

    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    if (input->GetKeyDown('W'))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('A'))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('E'))
        cameraNode_->Translate(Vector3::UP * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('Q'))
        cameraNode_->Translate(Vector3::DOWN * MOVE_SPEED * timeStep);
}


void Game::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
}


void Game::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    auto input = GetSubsystem<Input>();

    if (input->GetMouseButtonDown(MOUSEB_RIGHT))
    {
        if (hoveredNode_)
        {
            auto staticModel = hoveredNode_->GetComponent<StaticModel>();
            auto material = staticModel->GetMaterial(0);
            material->SetShaderParameter("OutlineEnable", false);
            hoveredNode_ = nullptr;
        }
        input->SetMouseVisible(false);
        MoveCamera(timeStep);
    }
    else
    {
        input->SetMouseVisible(true);
        if (hoveredNode_)
        {
            auto staticModel = hoveredNode_->GetComponent<StaticModel>();
            auto material = staticModel->GetMaterial(0);
            material->SetShaderParameter("OutlineEnable", false);
        }
        hoveredNode_ = Raycast();
        if (hoveredNode_)
        {
            auto staticModel = hoveredNode_->GetComponent<StaticModel>();
            auto material = staticModel->GetMaterial(0);
            material->SetShaderParameter("OutlineEnable", true);
        }
        if (input->GetMouseButtonPress(MOUSEB_LEFT) && hoveredNode_)
        {
            auto staticModel = hoveredNode_->GetComponent<StaticModel>();
            auto material = staticModel->GetMaterial(0);
            if (hoveredNode_->HasTag("Selected"))
            {
                hoveredNode_->RemoveTag("Selected");
                material->SetShaderParameter("MatDiffColor", Color::WHITE);
            }
            else
            {
                hoveredNode_->AddTag("Selected");
                material->SetShaderParameter("MatDiffColor", Color::RED);
            }
        }
    }

    if (input->GetKeyPress(KEY_F2))
        GetSubsystem<DebugHud>()->ToggleAll();
}


void Game::CreateUI()
{
    auto cache = GetSubsystem<ResourceCache>();
    auto xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    auto debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);

    auto ui = GetSubsystem<UI>();
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText("Use WASDQE keys to move,\n"
                             "LMB to select/deselect mushrooms,\n"
                             "RMB to rotate view");
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 20);
    instructionText->SetTextAlignment(HA_CENTER);
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
    instructionText->SetTextEffect(TextEffect::TE_STROKE);
}
