#ifndef _ANIMATED_DEMO_SCENE_h
#define _ANIMATED_DEMO_SCENE_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include <IntegerWorld.h>
#include "DemoSceneAssets.h"

using namespace IntegerWorld;


class AnimatedDemoScene : private TS::Task
{
public:
	static constexpr uint8_t ObjectsCount = 7
#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
		+Assets::Debug::FrustumDebugger::ObjectCount
#endif
		;

	// Worst case scenario for all objects.
	static constexpr uint16_t MaxDrawCallCount = Assets::Shapes::Sphere::TriangleCount
		+ Assets::Shapes::Star::TriangleCount
		+ Assets::Shapes::Cube::TriangleCount
		+ Assets::Shapes::Grid8x8::VertexCount;

private:
	enum class SceneLightsEnum : uint8_t
	{
		GlobalDirectional,
		PointLight1,
		PointLight2,
		EnumCount
	};

	static constexpr Rgb8::color_t Light1Color = Rgb8::RED;
	static constexpr Rgb8::color_t Light2Color = Rgb8::GREEN;
	static constexpr Rgb8::color_t Light3Color = Rgb8::BLUE;
	static constexpr Rgb8::color_t GlobalLightColor = 0xEDE9CB;
	static constexpr Rgb8::color_t AmbientLightColor = 0x18232D;

	static constexpr uint32_t FloorColorPeriodMicros = 3000000000;
	static constexpr uint32_t FlickerPeriodMicros = 6400000;
	static constexpr uint32_t FlashColorPeriodMicros = 3000000;
	static constexpr uint32_t ShapeColorPeriodMicros = 19000000;
	static constexpr uint32_t ShapeRotatePeriodMicros = 35000000;
	static constexpr uint32_t ShapeMovePeriodMicros = 15111111;

	static constexpr int16_t DistanceUnit = Assets::Shapes::SHAPE_UNIT;
	static constexpr int16_t BaseDistance = (VERTEX16_UNIT * 15) / 10;
	static constexpr int16_t ShapeMove = (DistanceUnit * 30) / 10;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 2;
	static constexpr uint16_t LightDimension = DistanceUnit / 3;

	static constexpr uint16_t LightMinDistance = 0;
	static constexpr uint16_t LightMaxDistance = DistanceUnit * 8;

	int16_t ShapeMoveX = (((int32_t)ShapeMove * 4) / 10);

private:
	// Light sources.
	light_source_t SceneLights[uint8_t(SceneLightsEnum::EnumCount)]{};

	// Billboard objects to track point light sources in the scene.
	BillboardObject<> Light1BillboardObject{};
	BillboardObject<> Light2BillboardObject{};

	// Custom billboard shaders for point light sources.
	Assets::Shaders::LightBillboardFragmentShader Billboard1Shader{};
	Assets::Shaders::LightBillboardFragmentShader Billboard2Shader{};

	// Shader for this scene, shared by all mesh objects.
	LightsShader SceneShader{};

	// Sphere has a diffuse material.
	Assets::Objects::SphereMeshLodObject ObjectSphere{};

	// Star has a reflective, slightly metallic material with animated color.
	Assets::Objects::StarMeshObject ObjectStar{};

	// Cube has a palleted diffuse color for each face.
	Assets::Objects::CubeMeshObject ObjectCube{};

	// Shared mesh objects shaders.
	TriangleFillShadedFragmentShader MeshShader{};
	TriangleFillZFragmentShader MeshZShader{};
	TriangleFillNormalFragmentShader MeshNormalShader{};

	// Floor has a custom shader to draw the diffuse floor object.
	Assets::Objects::FloorPointCloudObject ObjectFloor{};
	Assets::Shaders::FloorFragmentShader FloorShader{};

	// Simple screen fill background.
	FlatBackgroundObject Background{};
	BackgroundFlatFillShader BackgroundShader{};

	// Shared point objects shaders.
	PointZFragmentShader PointZShader{};
	PointPixelFixedColorFragmentShader<> PointNormalShader{};

	// Resumable animation trackers.
	uint32_t AnimationStart = 0;
	uint32_t AnimationPause = 0;

#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
	// Frustum debugger to visualize the camera frustum in the scene.
	Assets::Debug::FrustumDebugger FrustumVisualizer{};
#endif

public:
	AnimatedDemoScene(TS::Scheduler& scheduler)
		: TS::Task(4, TASK_FOREVER, &scheduler, false)
	{
	}

	bool Callback() final
	{
		const uint32_t timestamp = micros();
		AnimateObjects(timestamp - AnimationStart);

		return true;
	}

	bool Start(IEngineRenderer& engineRenderer, const int16_t width, const int16_t height)
	{
		// Clear all objects from the renderer.
		engineRenderer.ClearObjects();

		// Add all scene objects to the pipeline.
		if (!engineRenderer.AddObject(&Background)
			|| !engineRenderer.AddObject(&Light1BillboardObject)
			|| !engineRenderer.AddObject(&Light2BillboardObject)
			|| !engineRenderer.AddObject(&ObjectSphere)
			|| !engineRenderer.AddObject(&ObjectStar)
			|| !engineRenderer.AddObject(&ObjectCube)
			|| !engineRenderer.AddObject(&ObjectFloor))
		{
			SetAnimationEnabled(false);
			return false;
		}

		// Configure animation based on surface dimensions.
		ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * width) / height;

		// Setup lights.
		SceneLights[uint8_t(SceneLightsEnum::GlobalDirectional)] = DirectionalLightSource(GlobalLightColor,
			{ VERTEX16_UNIT, VERTEX16_UNIT, VERTEX16_UNIT }, UFRACTION16_1X / 4);

		SceneLights[uint8_t(SceneLightsEnum::PointLight1)] = PointLightSource(Light1Color,
			{ int16_t(-(ShapeMoveX * 3) / 10) , int16_t(-(DistanceUnit * 2) / 10) , int16_t((DistanceUnit * 15) / 10) },
			LightMinDistance, LightMaxDistance);

		SceneLights[uint8_t(SceneLightsEnum::PointLight2)] = PointLightSource(Light2Color * 0,
			{ int16_t((ShapeMoveX * 3) / 10) , int16_t(-(DistanceUnit * 2) / 10) , int16_t((DistanceUnit * 15) / 10) },
			LightMinDistance, LightMaxDistance);

		// Attach light sources to shader.
		SceneShader.SetLights(SceneLights, uint8_t(SceneLightsEnum::EnumCount));

		// Set the ambient color.
		SceneShader.AmbientLight = AmbientLightColor;

		// Lights track the camera position.
		SceneShader.CameraPosition = &engineRenderer.GetCameraControls()->Position;

		// Place the billboards on the lights' position.
		Light1BillboardObject.Translation = SceneLights[uint8_t(SceneLightsEnum::PointLight1)].Position;
		Light2BillboardObject.Translation = SceneLights[uint8_t(SceneLightsEnum::PointLight2)].Position;
		Light1BillboardObject.SetDimensions(LightDimension, LightDimension);
		Light2BillboardObject.SetDimensions(LightDimension, LightDimension);

		// Configure and place floor.
		PointNormalShader.Color = Rgb8::WHITE;
		FloorShader.Radius = MaxValue(1, MinValue(width, height) / 96);
		ObjectFloor.Resize = Scale16::GetFactor(uint8_t(32), uint8_t(1));
		ObjectFloor.Translation.x = Scale(ObjectFloor.Resize, int16_t(-DistanceUnit / 5));
		ObjectFloor.Translation.y = (DistanceUnit * 3) / 5;
		ObjectFloor.Translation.z = 0;
		ObjectFloor.Rotation.x = Trigonometry::ANGLE_90;

		// Set the light colors.
		SetLight1Enabled(true);
		SetLight2Enabled(true);
		SetLightGlobalEnabled(true);

		// Attach lights source to shader objects.
		Billboard1Shader.LightSource = &SceneLights[uint8_t(SceneLightsEnum::PointLight1)];
		Billboard2Shader.LightSource = &SceneLights[uint8_t(SceneLightsEnum::PointLight2)];

		// Configure object materials.
		ObjectStar.Material = material_t{ 0, UFRACTION8_1X / 4, UFRACTION8_1X, UFRACTION8_1X / 4 };
		ObjectSphere.Material = material_t{ 0, UFRACTION8_1X, 0,0 };

		// Configure background.
		Background.FragmentShader = &BackgroundShader;

		// Attach shaders to objects for rendering.
		SetSceneShader();
		SetPixelShader();

#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
		// Start the frustum visualizer.
		if (!FrustumVisualizer.Start(engineRenderer))
		{
			return false;
		}
#endif

		// Start the animation.
		SetAnimationEnabled(true);

		return true;
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
		// Start rainbow color pattern with HSV color.
		const ufraction16_t colorFraction = UFraction16::GetScalar((uint32_t)(timestamp % (ShapeColorPeriodMicros + 1)), ShapeColorPeriodMicros);
		ObjectStar.Color = Rgb8::ColorHsvFraction(colorFraction, UFRACTION16_1X, UFRACTION16_1X);

		// Continuous rotation on all 3 axis.
		const ufraction32_t xRotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (ShapeRotatePeriodMicros + 1)), ShapeRotatePeriodMicros);
		const ufraction32_t yRotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (((ShapeRotatePeriodMicros * 1) / 3) + 1)), (ShapeRotatePeriodMicros * 1) / 3);
		const ufraction32_t zRotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (((ShapeRotatePeriodMicros * 2) / 3) + 1)), (ShapeRotatePeriodMicros * 2) / 3);

		ObjectSphere.Rotation.x = Fraction(xRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectSphere.Rotation.y = Fraction(yRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectSphere.Rotation.z = Fraction(zRotateFraction, (uint16_t)ANGLE_RANGE);

		ObjectStar.Rotation.x = ObjectSphere.Rotation.x + GetAngle(120);
		ObjectStar.Rotation.y = ObjectSphere.Rotation.y + GetAngle(120);
		ObjectStar.Rotation.z = ObjectSphere.Rotation.z + GetAngle(120);;

		ObjectCube.Rotation.x = ObjectSphere.Rotation.x + GetAngle(240);
		ObjectCube.Rotation.y = ObjectSphere.Rotation.y + GetAngle(240);
		ObjectCube.Rotation.z = ObjectSphere.Rotation.z + GetAngle(240);

		// Circle animation.
		const ufraction32_t circleFraction = UFraction32::GetScalar((uint32_t)(timestamp % (ShapeMovePeriodMicros + 1)), ShapeMovePeriodMicros);
		const angle_t circleAngle = Fraction(circleFraction, ANGLE_RANGE);

		const fraction32_t xMoveFraction1 = Sine32(circleAngle);
		const fraction32_t xMoveFraction2 = Sine32((int32_t)circleAngle + GetAngle(120));
		const fraction32_t xMoveFraction3 = Sine32((int32_t)circleAngle + GetAngle(240));

		const fraction32_t zMoveFraction1 = Cosine32((int32_t)circleAngle);
		const fraction32_t zMoveFraction2 = Cosine32((int32_t)circleAngle + GetAngle(120));
		const fraction32_t zMoveFraction3 = Cosine32((int32_t)circleAngle + GetAngle(240));

		ObjectSphere.Translation.x = Fraction(xMoveFraction1, ShapeMoveX);
		ObjectStar.Translation.x = Fraction(xMoveFraction2, ShapeMoveX);
		ObjectCube.Translation.x = Fraction(xMoveFraction3, ShapeMoveX);

		ObjectSphere.Translation.z = BaseDistance + Fraction(zMoveFraction1, ShapeMoveZ);
		ObjectStar.Translation.z = BaseDistance + Fraction(zMoveFraction2, ShapeMoveZ);
		ObjectCube.Translation.z = BaseDistance + Fraction(zMoveFraction3, ShapeMoveZ);
	}

public:
	void CaptureViewFrustum()
	{
#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
		FrustumVisualizer.CaptureViewFrustum();
#endif
	}

	void SetSceneShader(const bool enabled = true)
	{
		if (enabled)
		{
			MeshShader.SceneShader = &SceneShader;
			FloorShader.SceneShader = &SceneShader;
		}
		else
		{
			MeshShader.SceneShader = nullptr;
			FloorShader.SceneShader = nullptr;
		}
	}

	void SetNoShader()
	{
		ObjectSphere.FragmentShader = nullptr;
		ObjectStar.FragmentShader = nullptr;
		ObjectCube.FragmentShader = nullptr;
		ObjectFloor.FragmentShader = nullptr;
		Light1BillboardObject.FragmentShader = nullptr;
		Light2BillboardObject.FragmentShader = nullptr;
	}

	void SetPixelShader()
	{
		ObjectSphere.FragmentShader = &MeshShader;
		ObjectStar.FragmentShader = &MeshShader;
		ObjectCube.FragmentShader = &MeshShader;
		ObjectFloor.FragmentShader = &FloorShader;

		Light1BillboardObject.FragmentShader = &Billboard1Shader;
		Light2BillboardObject.FragmentShader = &Billboard2Shader;

		// Set background color as half the ambient light.
		Background.Color = Rgb8::Color(Rgb8::Red(SceneShader.AmbientLight) >> 1,
			Rgb8::Green(SceneShader.AmbientLight) >> 1,
			Rgb8::Blue(SceneShader.AmbientLight) >> 1);
	}

	void SetZShader()
	{
		ObjectSphere.FragmentShader = &MeshZShader;
		ObjectStar.FragmentShader = &MeshZShader;
		ObjectCube.FragmentShader = &MeshZShader;
		ObjectFloor.FragmentShader = &PointZShader;

		Light1BillboardObject.FragmentShader = nullptr;
		Light2BillboardObject.FragmentShader = nullptr;

		Background.Color = Rgb8::BLACK;
	}

	void SetNormalShader()
	{
		ObjectSphere.FragmentShader = &MeshNormalShader;
		ObjectStar.FragmentShader = &MeshNormalShader;
		ObjectCube.FragmentShader = &MeshNormalShader;
		ObjectFloor.FragmentShader = &PointNormalShader;

		Light1BillboardObject.FragmentShader = nullptr;
		Light2BillboardObject.FragmentShader = nullptr;
		Background.Color = Rgb8::BLACK;

	}

	void SetLight1Enabled(const bool enabled)
	{
		if (enabled)
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLight1)].Color = Light1Color;
		}
		else
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLight1)].Color = 0;
		}
	}

	void SetLight2Enabled(const bool enabled)
	{
		if (enabled)
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLight2)].Color = Light2Color;
		}
		else
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLight2)].Color = 0;
		}
	}

	void SetLightGlobalEnabled(const bool enabled)
	{
		if (enabled)
		{
			SceneLights[uint8_t(SceneLightsEnum::GlobalDirectional)].Color = GlobalLightColor;
		}
		else
		{
			SceneLights[uint8_t(SceneLightsEnum::GlobalDirectional)].Color = 0;
		}
	}

	void SetAmbientShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		SceneShader.Ambient = enabled;
#endif
	}

	void SetEmissiveShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		SceneShader.Emissive = enabled;
#endif
	}

	void SetDiffuseShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		SceneShader.Diffuse = enabled;
#endif
	}

	void SetSpecularShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		SceneShader.Specular = enabled;
#endif
	}

	void SetAnimationEnabled(const bool enabled)
	{
		if (enabled != TS::Task::isEnabled())
		{
			const uint32_t timestamp = micros();

			if (enabled)
			{
				const uint32_t skipped = timestamp - AnimationPause;

				AnimationStart += skipped;
				TS::Task::enable();

				AnimateObjects(micros() - AnimationStart);
			}
			else
			{
				AnimationPause = timestamp;
				TS::Task::disable();
			}
		}
	}
};
#endif