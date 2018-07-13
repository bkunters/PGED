#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>


#include "dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

#include "d3dx11effect.h"

#include "Terrain.h"
#include "GameEffect.h"
#include "ConfigParser.h"
#include "Mesh.h"
#include "Enemy.h"
#include "SpriteRenderer.h"

#include <list>
#include <time.h>
#include <stdlib.h>

#include "debug.h"
#include <random>


// Help macros
#define DEG2RAD( a ) ( (a) * XM_PI / 180.f )

using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

// ConfigFile
ConfigParser							parseConf; // Initialized at InitApp()

// Camera
struct CAMERAPARAMS {
	float   fovy;
	float   aspect;
	float   nearPlane;
	float   farPlane;
}                                       g_cameraParams;
float                                   g_cameraMoveScaler = 1000.f;
float                                   g_cameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_camera;               // A first person camera
bool									g_cameraMovement = false;

// User Interface
CDXUTDialogResourceManager              g_dialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_settingsDlg;          // Device settings dialog
CDXUTTextHelper*                        g_txtHelper = NULL;
CDXUTDialog                             g_hud;                  // dialog for standard controls
CDXUTDialog                             g_sampleUI;             // dialog for sample specific controls

// ID3D11InputLayout*                      g_terrainVertexLayout; // Describes the structure of the vertex buffer to the input assembler stage
std::map<std::string, Mesh*>			g_Meshes;


bool                                    g_terrainSpinning = false;
XMMATRIX                                g_terrainWorld; // object- to world-space transformation


// Scene information
XMVECTOR                                g_lightDir;
Terrain									g_terrain;

GameEffect								g_gameEffect; // CPU part of Shader

// Enemy Movement
std::list<Enemy*>						enemies;
float									g_SpawnTimer = 0.0f;
float									enemyRadiusBorder;
float									enemyRadiusOut;
float									enemyRadiusIn;

// SpriteRenderer
SpriteRenderer*							g_SpriteRenderer;

// Projectile Movement
Gun										gatling;
Gun										plasma;
std::list<GunProjectileVertex>			gatlingProjectiles;
std::list<GunProjectileVertex>			plasmaProjectiles;
float									g_gatlingTimer = 0.0f;
float									g_plasmaTimer = 0.0f;

// Explosion Movement
Explosion								explosion;
std::list<ExplosionState>				explosions;
ExplosionParticle						explosionParticle;
std::list<ParticleState>				explosionParticles;

// random generator
std::default_random_engine generator;
std::normal_distribution<float> distribute;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *, UINT , const CD3D11EnumDeviceInfo *,
                                       DXGI_FORMAT, bool, void* );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext );

void InitApp();
void DeinitApp();
void RenderText();

void ReleaseShader();
HRESULT ReloadShader(ID3D11Device* pd3dDevice);

bool intersects(Enemy e, GunProjectileVertex p);
void spawnParticleSystem(XMFLOAT3 pos);

bool intersects(Enemy e, GunProjectileVertex p) 
{
	XMVECTOR enemyPos = XMVectorSet(e.getPosition().x, e.getPosition().y, e.getPosition().z, 1.0f);
	XMVECTOR projectilePos = XMVectorSet(p.projectile.position.x, p.projectile.position.y, p.projectile.position.z, 1.0f);
	XMVECTOR distanceVector = XMVectorSubtract(enemyPos, projectilePos);
	float dot = XMVectorGetX(XMVector3Dot(distanceVector, distanceVector));
	if (dot > (e.getSize() + p.projectile.radius) * (e.getSize() + p.projectile.radius))
		return false;
	else
		return true;
}

void spawnParticleSystem(XMFLOAT3 pos) 
{
	for (int i = 0; i < explosionParticle.amount; i++) 
	{
		XMFLOAT3 direction(distribute(generator), distribute(generator), distribute(generator));
		// use rejection sampling
		while (direction.x * direction.x + direction.y * direction.y + direction.z * direction.z > 1.0f)
		{
			direction = XMFLOAT3(distribute(generator), distribute(generator), distribute(generator));
		}
		XMVECTOR dirVector = XMVectorSet(direction.x, direction.y, direction.z, 1.0f);
		XMVector3Normalize(dirVector);
		direction = XMFLOAT3(XMVectorGetX(dirVector), XMVectorGetY(dirVector), XMVectorGetZ(dirVector));
		float speed = explosionParticle.min + (float)rand() / (float)(RAND_MAX / explosionParticle.max);
		XMFLOAT3 velocity(direction.x * speed, direction.y * speed, direction.z * speed);
		SpriteVertex v = { pos, 2, 2, 0, 1 };
		ParticleState p = { v, velocity };
		explosionParticles.push_back(p);
	}
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Old Direct3D Documentation:
    // Start > All Programs > Microsoft DirectX SDK (June 2010) > Windows DirectX Graphics Documentation

    // DXUT Documentaion:
    // Start > All Programs > Microsoft DirectX SDK (June 2010) > DirectX Documentation for C++ : The DirectX Software Development Kit > Programming Guide > DXUT
	
    // New Direct3D Documentaion (just for reference, use old documentation to find explanations):
    // http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466%28v=vs.85%29.aspx


    // Initialize COM library for windows imaging components
    /*HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr != S_OK)
    {
        MessageBox(0, L"Error calling CoInitializeEx", L"Error", MB_OK | MB_ICONERROR);
        exit(-1);
    }*/


    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    //DXUTSetIsInGammaCorrectMode(false);

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"MyGame" ); // You may change the title

    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );

    DXUTMainLoop(); // Enter into the DXUT render loop
	DXUTShutdown();
	DeinitApp();

    return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	srand(time(NULL));
    HRESULT hr;
    WCHAR path[MAX_PATH];

    // Parse the config file

    V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	char pathA[MAX_PATH];
	size_t size;
	wcstombs_s(&size, pathA, path, MAX_PATH);
	// last step: load the config file
	parseConf.load(pathA);

	// set spinning
	g_terrainSpinning = parseConf.getSpinning();
	// set radius for enemy movement
	enemyRadiusIn = parseConf.getTerrainWidth() / 4.0f;
	enemyRadiusOut = (parseConf.getTerrainWidth() / 2.0f) * 1.5f;
	enemyRadiusBorder = (parseConf.getTerrainWidth() / 2.0f) * 2.0f;

	std::map<std::string, std::vector<std::string>> meshes = parseConf.getMeshes();
	// create the Meshes
	for (auto it = meshes.begin(); it != meshes.end(); it++) 
	{
		std::string meshName = it->first;
		std::vector<std::string> files = it->second;
		g_Meshes[meshName] = new Mesh(files[0], files[1], files[2], files[3]);
	}

	// create the SpriteRenderer
	g_SpriteRenderer = new SpriteRenderer(parseConf.getSprites());

	// set guns
	gatling = parseConf.getGatling();
	plasma = parseConf.getPlasma();

	// set explosion
	explosion = parseConf.getExplosion();
	// set explosion particle
	explosionParticle = parseConf.getExplosionParticle();

	// set random number generator
	distribute = std::normal_distribution<float>(0.0f, 0.6f);
	generator = std::default_random_engine(time(NULL));

    // Intialize the user interface

    g_settingsDlg.Init( &g_dialogResourceManager );
    g_hud.Init( &g_dialogResourceManager );
    g_sampleUI.Init( &g_dialogResourceManager );

    g_hud.SetCallback( OnGUIEvent );
    int iY = 30;
    int iYo = 26;
    g_hud.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
    g_hud.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
    g_hud.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

	g_hud.AddButton (IDC_RELOAD_SHADERS, L"Reload shaders (F5)", 0, iY += 24, 170, 22, VK_F5);
    
    g_sampleUI.SetCallback( OnGUIEvent ); iY = 10;
    iY += 24;
    g_sampleUI.AddCheckBox( IDC_TOGGLESPIN, L"Toggle Spinning", 0, iY += 24, 125, 22, g_terrainSpinning );   
}

void DeinitApp() 
{
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) 
	{
		std::string meshName = it->first;
		SAFE_DELETE(g_Meshes[meshName]);
	}
	g_Meshes.clear();
	for (auto it = enemies.begin(); it != enemies.end(); it++) 
	{
		SAFE_DELETE(*it);
	}
	enemies.clear();
	SAFE_DELETE(g_SpriteRenderer);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_txtHelper->Begin();
    g_txtHelper->SetInsertionPos( 5, 5 );
    g_txtHelper->SetForegroundColor(XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
    g_txtHelper->DrawTextLine( DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
    g_txtHelper->DrawTextLine( DXUTGetDeviceStats() );
    g_txtHelper->End();
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *, UINT, const CD3D11EnumDeviceInfo *,
        DXGI_FORMAT, bool, void* )
{
    return true;
}

//--------------------------------------------------------------------------------------
// Specify the initial device settings
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pDeviceSettings);
	UNREFERENCED_PARAMETER(pUserContext);

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
        {
            DXUTDisplaySwitchingToREFWarning();
        }
    }
    //// Enable anti aliasing
    pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
    pDeviceSettings->d3d11.sd.SampleDesc.Quality = 1;

    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pBackBufferSurfaceDesc);
	UNREFERENCED_PARAMETER(pUserContext);

    HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // http://msdn.microsoft.com/en-us/library/ff476891%28v=vs.85%29
    V_RETURN( g_dialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_settingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_txtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, 15 );

    V_RETURN( ReloadShader(pd3dDevice) );

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "SV_POSITION",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",         0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",       0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// Create the input layout
    D3DX11_PASS_DESC pd;
	V_RETURN(g_gameEffect.pass0->GetDesc(&pd));
	//V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, pd.pIAInputSignature,
    //        pd.IAInputSignatureSize, g_TerrainVertexLayout ) );



	// Create the terrain

	V_RETURN(g_terrain.create(pd3dDevice, parseConf));

	// create the Meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) 
	{
		std::string meshName = it->first;
		V_RETURN(g_Meshes[meshName]->create(pd3dDevice));
	}

	// Define the input layout for Meshes
	Mesh::createInputLayout(pd3dDevice, g_gameEffect.meshPass1);

	// create the SpriteRenderer
	g_SpriteRenderer->create(pd3dDevice);

	// Initialize the camera
	XMVECTOR vEye = XMVectorSet(0.0f, g_terrain.getMaxHeight() * parseConf.getTerrainHeight() + 400.0f, 0.000000000000001f, 0.0f);   // Camera eye is here
	XMVECTOR vAt = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);               // ... facing at this position
	g_camera.SetViewParams(vEye, vAt); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
	
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);

    g_dialogResourceManager.OnD3D11DestroyDevice();
    g_settingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    //SAFE_RELEASE( g_terrainVertexLayout );

	// Destroy the terrain
	g_terrain.destroy();

	// Release the input layout for Meshes
	Mesh::destroyInputLayout();

	// Destroy the meshes
	for (auto it = g_Meshes.begin(); it != g_Meshes.end(); it++) 
	{
		std::string meshName = it->first;
		g_Meshes[meshName]->destroy();
	}

	// Destroy the SpriteRenderer
	g_SpriteRenderer->destroy();

    SAFE_DELETE( g_txtHelper );
    ReleaseShader();
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pSwapChain);
	UNREFERENCED_PARAMETER(pUserContext);

    HRESULT hr;
    
    // Intialize the user interface

    V_RETURN( g_dialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_settingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    g_hud.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_hud.SetSize( 170, 170 );
    g_sampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_sampleUI.SetSize( 170, 300 );

    // Initialize the camera

    g_cameraParams.aspect = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_cameraParams.fovy = 0.785398f;
    g_cameraParams.nearPlane = 1.f;
    g_cameraParams.farPlane = 5000.f;

    g_camera.SetProjParams(g_cameraParams.fovy, g_cameraParams.aspect, g_cameraParams.nearPlane, g_cameraParams.farPlane);
	g_camera.SetEnablePositionMovement(false);
	g_camera.SetRotateButtons(true, false, false);
	g_camera.SetScalers( g_cameraRotateScaler, g_cameraMoveScaler );
	g_camera.SetDrag( true );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);
    g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Loads the effect from file
// and retrieves all dependent variables
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice)
{
    assert(pd3dDevice != NULL);

    HRESULT hr;

    ReleaseShader();
	V_RETURN(g_gameEffect.create(pd3dDevice));
	// reload the Shader for Sprites
	g_SpriteRenderer->reloadShader(pd3dDevice);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	g_gameEffect.destroy();
	// release the Shader for Sprites
	g_SpriteRenderer->releaseShader();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);

    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_dialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_settingsDlg.IsActive() )
    {
        g_settingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_hud.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_sampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
        
    // Use the mouse weel to control the movement speed
    if(uMsg == WM_MOUSEWHEEL) {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_cameraMoveScaler *= (1 + zDelta / 500.0f);
        if (g_cameraMoveScaler < 0.1f)
          g_cameraMoveScaler = 0.1f;
        g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
    }

    // Pass all remaining windows messages to camera so it can respond to user input
    g_camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	UNREFERENCED_PARAMETER(nChar);
	UNREFERENCED_PARAMETER(bKeyDown);
	UNREFERENCED_PARAMETER(bAltDown);
	UNREFERENCED_PARAMETER(pUserContext);
	// Enable position movement when the C-key is pressed
	if (nChar == 'C' && bKeyDown) { g_camera.SetEnablePositionMovement(!g_cameraMovement); g_cameraMovement = !g_cameraMovement; }
	// Fire gatling if pressed 'Z'
	else if (nChar == 'Z') 
	{ 
		gatling.firing = bKeyDown;
	}
	// Fire plasma if pressed 'X'
	else if (nChar == 'X') 
	{ 
		plasma.firing = bKeyDown;
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	UNREFERENCED_PARAMETER(nEvent);
	UNREFERENCED_PARAMETER(pControl);
	UNREFERENCED_PARAMETER(pUserContext);

    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_settingsDlg.SetActive( !g_settingsDlg.IsActive() ); break;
        case IDC_TOGGLESPIN:
            g_terrainSpinning = g_sampleUI.GetCheckBox( IDC_TOGGLESPIN )->GetChecked();
            break;
		case IDC_RELOAD_SHADERS:
			ReloadShader(DXUTGetD3D11Device ());
			break;
    }
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_SpawnTimer -= fElapsedTime;
	// create new enemy if enough time is passed
	if (g_SpawnTimer <= 0.0f) 
	{
		g_SpawnTimer += parseConf.getSpawnInfo().intervall;
		std::vector<std::string> typeList = parseConf.getEnemyTypesList();
		std::map<std::string, EnemyType> enemyTypes = parseConf.getEnemyTypes();
		int randomType = rand() % typeList.size();
		EnemyType type = enemyTypes[typeList[randomType]];
		float randOut = (float)rand() / (float)(RAND_MAX / (XM_PI * 2));
		float randIn = (float)rand() / (float)(RAND_MAX / (XM_PI * 2));
		float randHeightConstant = parseConf.getSpawnInfo().minHeight + (float)rand() / (float)(RAND_MAX / parseConf.getSpawnInfo().maxHeight);
		float randHeight = 300.0f + g_terrain.getMaxHeight() * randHeightConstant * parseConf.getTerrainHeight();
		Vector3 posIn = Vector3(enemyRadiusIn * std::sin(randIn), randHeight, enemyRadiusIn * std::cos(randIn));
		Vector3 posOut = Vector3(enemyRadiusOut * std::sin(randOut), randHeight, enemyRadiusOut * std::cos(randOut));
		XMVECTOR velocity = type.speed * XMVector4Normalize(XMVectorSet(posIn.x - posOut.x, 0, posIn.z - posOut.z, 0));
		Vector3 v = Vector3(XMVectorGetX(velocity), XMVectorGetY(velocity), XMVectorGetZ(velocity));
		Enemy* newEnemy = new Enemy(typeList[randomType], type.speed, posOut, v, type.size, type.hitpoints);
		enemies.push_back(newEnemy);
	}

	// move enemy, also remove enemy if they leave
	for (auto it = enemies.begin(); it != enemies.end();)
	{
		Enemy* obj_ptr = *it;
		Enemy& obj = **it;
		if (std::sqrt(std::pow(obj.getPosition().x, 2.0f) + std::pow(obj.getPosition().z, 2.0f)) > enemyRadiusBorder)
		{
			auto it_remove = it;
			// free memory
			delete *it_remove;
			it++;
			enemies.erase(it_remove);
		}
		else 
		{
			obj.setPosition(obj.getPosition() + obj.getVelocity() * fElapsedTime);
			it++;
		}
	}

	// spawn projectile for each gun if not on cooldown and is firing
	if(g_gatlingTimer >= 0.0f)
		g_gatlingTimer -= fElapsedTime;
	if (g_plasmaTimer >= 0.0f)
		g_plasmaTimer -= fElapsedTime;

	if (g_gatlingTimer < 0.0f && gatling.firing) 
	{
		XMMATRIX world = g_camera.GetWorldMatrix();
		g_gatlingTimer = gatling.cooldown;
		XMVECTOR pos = XMVectorSet(gatling.x, gatling.y, gatling.z, 1.0f);
		pos = XMVector4Transform(pos, world);
		XMFLOAT3 initPos(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
		XMVECTOR direction = g_camera.GetWorldAhead();
		direction = XMVector3Normalize(direction);
		XMFLOAT3 velocity(gatling.speed * XMVectorGetX(direction), gatling.speed * XMVectorGetY(direction), gatling.speed * XMVectorGetZ(direction));
		float radius = gatling.sprite_radius;
		UINT texture_id = gatling.sprite_tex_index;
		SpriteVertex projectileSprite = { initPos, radius, texture_id, 0.0f, 1.0f };
		GunProjectileVertex projectile = { projectileSprite, 0.0f, 4.0f, velocity};
		gatlingProjectiles.push_back(projectile);
	}

	if (g_plasmaTimer < 0.0f && plasma.firing) 
	{
		XMMATRIX world = g_camera.GetWorldMatrix();
		g_plasmaTimer = plasma.cooldown;
		XMVECTOR pos = XMVectorSet(plasma.x, plasma.y, plasma.z, 1.0f);
		pos = XMVector4Transform(pos, world);
		XMFLOAT3 initPos(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
		XMVECTOR direction = g_camera.GetWorldAhead();
		direction = XMVector3Normalize(direction);
		XMFLOAT3 velocity(plasma.speed * XMVectorGetX(direction), plasma.speed * XMVectorGetY(direction), plasma.speed * XMVectorGetZ(direction));
		float radius = plasma.sprite_radius;
		UINT texture_id = plasma.sprite_tex_index;
		SpriteVertex projectileSprite = { initPos, radius, texture_id, 0.0f, 1.0f };
		GunProjectileVertex projectile = { projectileSprite, 0.0f, 4.0f, velocity };
		plasmaProjectiles.push_back(projectile);
	}

	// move projectiles
	
	// check gatling
	for (auto it = gatlingProjectiles.begin(); it != gatlingProjectiles.end();) 
	{
		GunProjectileVertex& vertex = *it;
		if (vertex.lifeTime >= vertex.maxLifeTime) 
		{
			auto it_remove = it;
			it++;
			gatlingProjectiles.erase(it_remove);
		}
		else 
		{
			vertex.lifeTime += fElapsedTime;
			// update velocity
			XMFLOAT3 velocity_old = vertex.velocity;
			XMFLOAT3 velocity_new(velocity_old.x, velocity_old.y - fElapsedTime * gatling.gravity, velocity_old.z);
			vertex.velocity = velocity_new;
			// update position
			XMFLOAT3 pos_old = vertex.projectile.position;
			XMFLOAT3 pos_new(pos_old.x + fElapsedTime * velocity_new.x, pos_old.y + fElapsedTime * velocity_new.y, 
				pos_old.z + fElapsedTime * velocity_new.z);
			SpriteVertex& alias = vertex.projectile;
			alias.position = pos_new;
			it++;
		}
	}

	// check plasma
	for (auto it = plasmaProjectiles.begin(); it != plasmaProjectiles.end();)
	{
		GunProjectileVertex& vertex = *it;
		if (vertex.lifeTime >= vertex.maxLifeTime)
		{
			auto it_remove = it;
			it++;
			plasmaProjectiles.erase(it_remove);
		}
		else
		{
			vertex.lifeTime += fElapsedTime;
			// update velocity
			XMFLOAT3 velocity_old = vertex.velocity;
			XMFLOAT3 velocity_new(velocity_old.x, velocity_old.y - fElapsedTime * plasma.gravity, velocity_old.z);
			vertex.velocity = velocity_new;
			// update position
			XMFLOAT3 pos_old = vertex.projectile.position;
			XMFLOAT3 pos_new(pos_old.x + fElapsedTime * velocity_new.x, pos_old.y + fElapsedTime * velocity_new.y,
				pos_old.z + fElapsedTime * velocity_new.z);
			SpriteVertex& alias = vertex.projectile;
			alias.position = pos_new;
			it++;
		}
	}

	// check projectile-enemy collision

	// check gatling
	for (auto it = gatlingProjectiles.begin(); it != gatlingProjectiles.end();) 
	{
		GunProjectileVertex& p = *it;
		bool projectileDestroyed = false;
		for (auto itEnemy = enemies.begin(); itEnemy != enemies.end();) 
		{
			Enemy& enemy = **itEnemy;
			if (intersects(enemy, p)) 
			{
				projectileDestroyed = true;
				float hp = enemy.getHitpoints();
				hp -= gatling.damage;
				enemy.setHitpoints(hp);
				if (hp <= 0.0f) 
				{
					// spawn explosion
					ExplosionState newExplosion = { p.projectile.position, explosion.texIndex, 0.0f, enemy.getSize() };
					explosions.push_back(newExplosion);
					// spawn particle system
					spawnParticleSystem(p.projectile.position);
					// delete enemy
					auto itEnemy_remove = itEnemy;
					itEnemy++;
					delete *itEnemy_remove;
					enemies.erase(itEnemy_remove);
				}
				break;
			}
			else { itEnemy++; }
		}
		if (projectileDestroyed) 
		{
			auto it_remove = it;
			it++;
			gatlingProjectiles.erase(it_remove);
		}
		else { it++; }
	}

	// check plasma
	for (auto it = plasmaProjectiles.begin(); it != plasmaProjectiles.end();)
	{
		GunProjectileVertex& p = *it;
		bool projectileDestroyed = false;
		for (auto itEnemy = enemies.begin(); itEnemy != enemies.end();)
		{
			Enemy& enemy = **itEnemy;
			if (intersects(enemy, p))
			{
				projectileDestroyed = true;
				float hp = enemy.getHitpoints();
				hp -= plasma.damage;
				enemy.setHitpoints(hp);
				if (hp <= 0.0f)
				{
					// spawn explosion
					ExplosionState newExplosion = { p.projectile.position, explosion.texIndex, 0.0f, enemy.getSize() };
					explosions.push_back(newExplosion);
					// spawn particle system
					spawnParticleSystem(p.projectile.position);
					// delete enemy
					auto itEnemy_remove = itEnemy;
					itEnemy++;
					delete *itEnemy_remove;
					enemies.erase(itEnemy_remove);
				}
				break;
			}
			else { itEnemy++; }
		}
		if (projectileDestroyed)
		{
			auto it_remove = it;
			it++;
			plasmaProjectiles.erase(it_remove);
		}
		else { it++; }
	}

	// update explosions, remove if time exceeds
	for (auto it = explosions.begin(); it != explosions.end();) 
	{
		ExplosionState& e = *it;
		e.timePassed += (fElapsedTime / explosion.lifetime);
		if (e.timePassed >= 1.0f) 
		{
			auto it_remove = it;
			it++;
			explosions.erase(it_remove);
		}
		else { it++; }
	}

	// update particles, remove if time exceeds
	for (auto it = explosionParticles.begin(); it != explosionParticles.end();) 
	{
		ParticleState& p = *it;
		if (p.particle.t >= 1.0f) 
		{
			auto it_remove = it;
			it++;
			explosionParticles.erase(it_remove);
		}
		else 
		{
			SpriteVertex& alias = p.particle;
			alias.t += (fElapsedTime / explosionParticle.lifetime);
			alias.alpha -= (fElapsedTime / explosionParticle.lifetime);
			// update velocity
			XMFLOAT3 velocity_old = p.velocity;
			XMFLOAT3 velocity_new(velocity_old.x, velocity_old.y - fElapsedTime * explosionParticle.gravityInfluence, velocity_old.z);
			p.velocity = velocity_new;
			// update position
			XMFLOAT3 pos_old = p.particle.position;
			XMFLOAT3 pos_new(pos_old.x + fElapsedTime * velocity_new.x, pos_old.y + fElapsedTime * velocity_new.y,
				pos_old.z + fElapsedTime * velocity_new.z);
			alias.position = pos_new;
			it++;
		}
	}

	UNREFERENCED_PARAMETER(pUserContext);
    // Update the camera's position based on user input 
    g_camera.FrameMove( fElapsedTime );
    
    // Initialize the terrain world matrix
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx
    
	// Start with identity matrix
    g_terrainWorld = XMMatrixIdentity();
	XMMATRIX worldScaling = XMMatrixScaling(parseConf.getTerrainWidth(), parseConf.getTerrainHeight(), parseConf.getTerrainDepth());
	g_terrainWorld = XMMatrixMultiply(g_terrainWorld, worldScaling);
    
    if( g_terrainSpinning ) 
    {
		// If spinng enabled, rotate the world matrix around the y-axis
        g_terrainWorld *= XMMatrixRotationY(30.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
    }

	// Set the light vector
    g_lightDir = XMVectorSet(1, 1, 1, 0); // Direction to the directional light in world space    
    g_lightDir = XMVector3Normalize(g_lightDir);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
        float fElapsedTime, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pd3dDevice);
	UNREFERENCED_PARAMETER(fTime);
	UNREFERENCED_PARAMETER(pUserContext);

    HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_settingsDlg.IsActive() )
    {
        g_settingsDlg.OnRender( fElapsedTime );
        return;
    }     

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	float clearColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
	// for black background uncomment, and comment the above clearColor definition
	// float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pd3dImmediateContext->ClearRenderTargetView( pRTV, clearColor );
        
	if(g_gameEffect.effect == NULL) {
        g_txtHelper->Begin();
        g_txtHelper->SetInsertionPos( 5, 5 );
        g_txtHelper->SetForegroundColor( XMVectorSet( 1.0f, 1.0f, 0.0f, 1.0f ) );
        g_txtHelper->DrawTextLine( L"SHADER ERROR" );
        g_txtHelper->End();
        return;
    }

    // Clear the depth stencil
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
    
    // Update variables that change once per frame
    XMMATRIX const view = g_camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
    XMMATRIX const proj = g_camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
    XMMATRIX worldViewProj = g_terrainWorld * view * proj;
	V(g_gameEffect.worldEV->SetMatrix( ( float* )&g_terrainWorld ));
	V(g_gameEffect.worldViewProjectionEV->SetMatrix( ( float* )&worldViewProj ));
	V(g_gameEffect.lightDirEV->SetFloatVector( ( float* )&g_lightDir ));
	// set WorldNormalsMatrix
	V(g_gameEffect.worldNormalsEV->SetMatrix( ( float* )&DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, g_terrainWorld))));

    // Set input layout
    pd3dImmediateContext->IASetInputLayout( nullptr );

	g_terrain.render(pd3dImmediateContext, g_gameEffect.pass0);
	
	// set camera position
	V(g_gameEffect.cameraPosWorldEV->SetFloatVector((float*)&g_camera.GetEyePt()));

	// transformation matrices for meshes
	XMMATRIX mTrans, mScale, mRotX, mRotY, mRotZ;
	const float  PI = 3.14159265358979f;
	// render cockpit objects
	std::vector<CockpitObject> cObjectsToRender = parseConf.getCockpitObjects();
	for (int i = 0; i < cObjectsToRender.size(); i++) 
	{
		std::string name = cObjectsToRender[i].name;
		mRotX = XMMatrixRotationX(cObjectsToRender[i].rotX * PI / 180.0f);
		mRotY = XMMatrixRotationY(cObjectsToRender[i].rotY * PI / 180.0f);
		mRotZ = XMMatrixRotationZ(cObjectsToRender[i].rotZ * PI / 180.0f);
		mScale = XMMatrixScaling(cObjectsToRender[i].scale, cObjectsToRender[i].scale, cObjectsToRender[i].scale);
		mTrans = XMMatrixTranslation(cObjectsToRender[i].transX, cObjectsToRender[i].transY, cObjectsToRender[i].transZ);
		XMMATRIX mWorld = mScale * mRotX * mRotY * mRotZ * mTrans *  g_camera.GetWorldMatrix();
		XMMATRIX mWorldViewProj = mWorld * view * proj;
		XMMATRIX mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		// set transformation matrices for mesh
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&mWorldNormals));
		// render mesh
		g_Meshes[name]->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV, g_gameEffect.specularEV, g_gameEffect.glowEV);
	}
	// render ground objects
	std::vector<GroundObject> gObjectsToRender = parseConf.getGroundObjects();
	for (int i = 0; i < gObjectsToRender.size(); i++)
	{
		std::string name = gObjectsToRender[i].name;
		mRotX = XMMatrixRotationX(gObjectsToRender[i].rotX * PI / 180.0f);
		mRotY = XMMatrixRotationY(gObjectsToRender[i].rotY * PI / 180.0f);
		mRotZ = XMMatrixRotationZ(gObjectsToRender[i].rotZ * PI / 180.0f);
		mScale = XMMatrixScaling(gObjectsToRender[i].scale, gObjectsToRender[i].scale, gObjectsToRender[i].scale);
		mTrans = XMMatrixTranslation(gObjectsToRender[i].transX, gObjectsToRender[i].transY, gObjectsToRender[i].transZ);
		XMMATRIX mWorld = mScale * mRotX * mRotY * mRotZ * mTrans;
		XMMATRIX mWorldViewProj = mWorld * view * proj;
		XMMATRIX mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		// set transformation matrices for mesh
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&mWorldNormals));
		// render mesh
		g_Meshes[name]->render(pd3dImmediateContext, g_gameEffect.meshPass1, g_gameEffect.diffuseEV, g_gameEffect.specularEV, g_gameEffect.glowEV);
	}
	// render enemy ships
	std::map<std::string, EnemyType> types = parseConf.getEnemyTypes();
	for (auto it = enemies.begin(); it != enemies.end(); it++) 
	{
		Enemy* obj_ptr = *it;
		Enemy obj_copy = *obj_ptr;
		Vector3 d = obj_copy.getVelocity();
		Vector3 p = obj_copy.getPosition();
		XMVECTOR direction = XMVector4Normalize(XMVectorSet(d.x, d.y, d.z, 0));
		XMVECTOR position = XMVectorSet(p.x, p.y, p.z, 0);
		// calculate angle alpha to face
		float alpha = std::atan2(XMVectorGetZ(direction), XMVectorGetX(direction));
		// rotation is counter clockwise
		XMMATRIX animRotY = XMMatrixRotationY(-alpha);
		XMMATRIX animTrans = XMMatrixTranslation(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position));
		mRotX = XMMatrixRotationX(types[obj_copy.getType()].rotX * XM_PI / 180.0f);
		mRotY = XMMatrixRotationY(types[obj_copy.getType()].rotY * XM_PI / 180.0f);
		mRotZ = XMMatrixRotationZ(types[obj_copy.getType()].rotZ * XM_PI / 180.0f);
		mTrans = XMMatrixTranslation(types[obj_copy.getType()].transX, types[obj_copy.getType()].transY, types[obj_copy.getType()].transZ);
		mScale = XMMatrixScaling(types[obj_copy.getType()].scale, types[obj_copy.getType()].scale, types[obj_copy.getType()].scale);
		XMMATRIX mObject = mScale * mRotX * mRotY * mRotZ * mTrans;
		XMMATRIX mAnimation = animRotY * animTrans;
		XMMATRIX mWorld = mObject * mAnimation;
		XMMATRIX mWorldViewProj = mWorld * view * proj;
		XMMATRIX mWorldNormals = XMMatrixTranspose(XMMatrixInverse(nullptr, mWorld));
		// set matrices for enemy
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldNormalsEV->SetMatrix((float*)&mWorldNormals));
		// render
		g_Meshes[types[obj_copy.getType()].mesh]->render(pd3dImmediateContext, g_gameEffect.meshPass1, 
			g_gameEffect.diffuseEV, g_gameEffect.specularEV, g_gameEffect.glowEV);
	}

	// render projectiles/sprites
	struct VertexAndDistance
	{
		SpriteVertex vertex;
		float distance;
	};
	std::vector<SpriteVertex> sprites;
	// pre-compute camera distances
	std::vector<VertexAndDistance> vertexAndDistances;
	XMVECTOR cameraDirection = g_camera.GetWorldAhead();
	for (auto it = gatlingProjectiles.begin(); it != gatlingProjectiles.end(); it++) 
	{
		GunProjectileVertex vertex = *it;
		XMVECTOR pos = XMVectorSet(vertex.projectile.position.x, vertex.projectile.position.y, vertex.projectile.position.z, 1.0f);
		float dotProduct = XMVectorGetX(XMVector3Dot(pos, cameraDirection));
		vertexAndDistances.push_back({ vertex.projectile, dotProduct });
	}
	for (auto it = plasmaProjectiles.begin(); it != plasmaProjectiles.end(); it++) 
	{
		GunProjectileVertex vertex = *it;
		XMVECTOR pos = XMVectorSet(vertex.projectile.position.x, vertex.projectile.position.y, vertex.projectile.position.z, 1.0f);
		float dotProduct = XMVectorGetX(XMVector3Dot(pos, cameraDirection));
		vertexAndDistances.push_back({ vertex.projectile, dotProduct });
	}
	for (auto it = explosions.begin(); it != explosions.end(); it++) 
	{
		ExplosionState e = *it;
		XMVECTOR pos = XMVectorSet(e.position.x, e.position.y, e.position.z, 1.0f);
		float dotProduct = XMVectorGetX(XMVector3Dot(pos, cameraDirection));
		SpriteVertex vertex = { e.position, e.size, explosion.texIndex, e.timePassed, 1.0f - e.timePassed };
		vertexAndDistances.push_back({ vertex, dotProduct });
	}
	for (auto it = explosionParticles.begin(); it != explosionParticles.end(); it++) 
	{
		ParticleState& p = *it;
		XMVECTOR pos = XMVectorSet(p.particle.position.x, p.particle.position.y, p.particle.position.z, 1.0f);
		float dotProduct = XMVectorGetX(XMVector3Dot(pos, cameraDirection));
		vertexAndDistances.push_back({ p.particle, dotProduct });
	}
	if (vertexAndDistances.size() > 0) 
	{
		auto sort = [](const VertexAndDistance& a, const VertexAndDistance& b) { return a.distance >= b.distance; };
		std::sort(vertexAndDistances.begin(), vertexAndDistances.end(), sort);
		for (int i = 0; i < vertexAndDistances.size(); i++) 
		{
			sprites.push_back(vertexAndDistances[i].vertex);
		}
		g_SpriteRenderer->renderSprites(pd3dImmediateContext, sprites, g_camera);
	}

    
    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    V(g_hud.OnRender( fElapsedTime ));
    V(g_sampleUI.OnRender( fElapsedTime ));
    RenderText();
    DXUT_EndPerfEvent();

    static DWORD dwTimefirst = GetTickCount();
    if ( GetTickCount() - dwTimefirst > 5000 )
    {    
        OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        OutputDebugString( L"\n" );
        dwTimefirst = GetTickCount();
    }
}
