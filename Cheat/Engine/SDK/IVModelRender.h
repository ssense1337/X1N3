#pragma once

#include "SDK.h"

#define STUDIO_SHADOWDEPTHTEXTURE		0x40000000

namespace SDK
{
	struct studiohwdata_t;
	struct StudioDecalHandle_t;
	class CStudioHdr;
	struct DrawModelInfo_t;
	class IMatRenderContext;
	struct MaterialLightingState_t;
	class ITexture;
	struct ColorMeshInfo_t;

	FORWARD_DECLARE_HANDLE(LightCacheHandle_t);

	FORWARD_DECLARE_HANDLE(memhandle_t);
	typedef memhandle_t DataCacheHandle_t;

	struct DrawModelState_t
	{
		studiohdr_t* m_pStudioHdr;
		studiohwdata_t* m_pStudioHWData;
		IClientRenderable* m_pRenderable;
		const matrix3x4_t* m_pModelToWorld;
		StudioDecalHandle_t* m_decals;
		int						m_drawFlags;
		int						m_lod;
	};

	typedef unsigned short ModelInstanceHandle_t;

	enum
	{
		MODEL_INSTANCE_INVALID = (ModelInstanceHandle_t)~0
	};

	struct ModelRenderInfo_t
	{
		Vector origin;
		QAngle angles;
		char pad[0x4];
		IClientRenderable* pRenderable;
		const model_t* pModel;
		const matrix3x4_t* pModelToWorld;
		const matrix3x4_t* pLightingOffset;
		const Vector* pLightingOrigin;
		int flags;
		int entity_index;
		int skin;
		int body;
		int hitboxset;
		ModelInstanceHandle_t instance;

		ModelRenderInfo_t()
		{
			pModelToWorld = NULL;
			pLightingOffset = NULL;
			pLightingOrigin = NULL;
		}
	};

	struct StaticPropRenderInfo_t
	{
		const matrix3x4_t* pModelToWorld;
		const model_t* pModel;
		IClientRenderable* pRenderable;
		Vector* pLightingOrigin;
		ModelInstanceHandle_t	instance;
		uint8					skin;
		uint8					alpha;
	};

	struct LightingQuery_t
	{
		Vector m_LightingOrigin;
		ModelInstanceHandle_t m_InstanceHandle;
		bool m_bAmbientBoost;
	};

	struct StaticLightingQuery_t : public LightingQuery_t
	{
		IClientRenderable* m_pRenderable;
	};

	enum OverrideType_t
	{
		OVERRIDE_NORMAL = 0,
		OVERRIDE_BUILD_SHADOWS,
		OVERRIDE_DEPTH_WRITE,
	};

	enum
	{
		ADDDECAL_TO_ALL_LODS = -1
	};

	namespace TABLE
	{
		namespace IVModelRender
		{
			enum
			{
				DrawModelExecute = 21,
			};
		}
	}

	class IVModelRender
	{
	public:
		virtual int	DrawModel(int flags, IClientRenderable* pRenderable, ModelInstanceHandle_t instance, int entity_index, const model_t* model, Vector const& origin, QAngle const& angles, int skin, int body, int hitboxset, const matrix3x4_t* modelToWorld = NULL, const matrix3x4_t* pLightingOffset = NULL) = 0;
		// This causes a material to be used when rendering the model instead 
		// of the materials the model was compiled with

		void ForcedMaterialOverride2(IMaterial* material, OverrideType_t type = OVERRIDE_NORMAL, int idk = NULL)
		{
			typedef void(__thiscall* Fn)(void*, IMaterial*, OverrideType_t, int);
			return call_vfunc<Fn>(this, 1)(this, material, type, idk);
		}

		virtual void	ForcedMaterialOverride(IMaterial* newMaterial, OverrideType_t nOverrideType = OVERRIDE_NORMAL, int iUnknown = 0) = 0;
		virtual bool	IsForcedMaterialOverride(void) = 0;
		virtual void	SetViewTarget(const CStudioHdr* pStudioHdr, int nBodyIndex, const Vector& target) = 0;
		// Creates, destroys instance data to be associated with the model
		virtual ModelInstanceHandle_t CreateInstance(IClientRenderable* pRenderable, LightCacheHandle_t* pCache = NULL) = 0;
		virtual void DestroyInstance(ModelInstanceHandle_t handle) = 0;
		// Associates a particular lighting condition with a model instance handle.
		// FIXME: This feature currently only works for static props. To make it work for entities, etc.,
		// we must clean up the lightcache handles as the model instances are removed.
		// At the moment, since only the static prop manager uses this, it cleans up all LightCacheHandles 
		// at level shutdown.
		virtual void SetStaticLighting(ModelInstanceHandle_t handle, LightCacheHandle_t* pHandle) = 0;
		virtual LightCacheHandle_t GetStaticLighting(ModelInstanceHandle_t handle) = 0;
		// moves an existing InstanceHandle to a nex Renderable to keep decals etc. Models must be the same
		virtual bool ChangeInstance(ModelInstanceHandle_t handle, IClientRenderable* pRenderable) = 0;
		// Creates a decal on a model instance by doing a planar projection
		// along the ray. The material is the decal material, the radius is the
		// radius of the decal to create.
		virtual void AddDecal(ModelInstanceHandle_t handle, Ray_t const& ray, Vector const& decalUp, int decalIndex, int body, bool noPokeThru = false, int maxLODToDecal = ADDDECAL_TO_ALL_LODS) = 0;
		// Removes all the decals on a model instance
		virtual void RemoveAllDecals(ModelInstanceHandle_t handle) = 0;
		virtual bool ModelHasDecals(ModelInstanceHandle_t handle) = 0;
		// Remove all decals from all models
		virtual void RemoveAllDecalsFromAllModels() = 0;
		// Shadow rendering, DrawModelShadowSetup returns the address of the bone-to-world array, NULL in case of error
		virtual matrix3x4_t* DrawModelShadowSetup(IClientRenderable* pRenderable, int body, int skin, DrawModelInfo_t* pInfo, matrix3x4_t* pCustomBoneToWorld = NULL) = 0;
		virtual void DrawModelShadow(IClientRenderable* pRenderable, const DrawModelInfo_t& info, matrix3x4_t* pCustomBoneToWorld = NULL) = 0;
		// This gets called when overbright, etc gets changed to recompute static prop lighting.
		virtual bool RecomputeStaticLighting(ModelInstanceHandle_t handle) = 0;
		virtual void ReleaseAllStaticPropColorData(void) = 0;
		virtual void RestoreAllStaticPropColorData(void) = 0;
		// Extended version of drawmodel
		virtual int	DrawModelEx(ModelRenderInfo_t& pInfo) = 0;
		virtual int	DrawModelExStaticProp(ModelRenderInfo_t& pInfo) = 0;
		virtual bool DrawModelSetup(ModelRenderInfo_t& pInfo, DrawModelState_t* pState, matrix3x4_t** ppBoneToWorldOut) = 0;
		virtual void DrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld = NULL) = 0;
		// Sets up lighting context for a point in space
		virtual void SetupLighting(const Vector& vecCenter) = 0;
		// doesn't support any debug visualization modes or other model options, but draws static props in the
		// fastest way possible
		virtual int DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth) = 0;
		// Allow client to override lighting state
		virtual void SuppressEngineLighting(bool bSuppress) = 0;
		virtual void SetupColorMeshes(int nTotalVerts) = 0;
		// Sets up lighting context for a point in space, with smooth interpolation per model.
		// Passing MODEL_INSTANCE_INVALID as a handle is equivalent to calling SetupLighting.
		virtual void SetupLightingEx(const Vector& vecCenter, ModelInstanceHandle_t handle) = 0;
		// Finds the brightest light source illuminating a point. Returns false if there isn't any.
		virtual bool GetBrightestShadowingLightSource(const Vector& vecCenter, Vector& lightPos, Vector& lightBrightness, bool bAllowNonTaggedLights) = 0;
		// Computes lighting state for an array of lighting requests
		virtual void ComputeLightingState(int nCount, const LightingQuery_t* pQuery, MaterialLightingState_t* pState, ITexture** ppEnvCubemapTexture) = 0;
		// Gets an array of decal handles given model instances
		virtual void GetModelDecalHandles(StudioDecalHandle_t* pDecals, int nDecalStride, int nCount, const ModelInstanceHandle_t* pHandles) = 0;
		// Computes lighting state for an array of lighting requests for renderables which use static lighting
		virtual void ComputeStaticLightingState(int nCount, const StaticLightingQuery_t* pQuery, MaterialLightingState_t* pState, MaterialLightingState_t* pDecalState, ColorMeshInfo_t** ppStaticLighting, ITexture** ppEnvCubemapTexture, DataCacheHandle_t* pColorMeshHandles) = 0;
		// Cleans up lighting state. Must be called after the draw call that uses
		// the color meshes return from ComputeStaticLightingState has been issued
		virtual void CleanupStaticLightingState(int nCount, DataCacheHandle_t* pColorMeshHandles) = 0;
	};

	template <class T>
	static T GetFunctionTARDasshloe(void* instance, int index)
	{
		const auto vtable = *static_cast<void***>(instance);
		return reinterpret_cast<T>(vtable[index]);
	}
	class IMaterialVar
	{
		void SetVectorInternal(const float x, const float y)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, float, float)>(this, 10)(this, x, y);
		}

		void SetVectorInternal(const float x, const float y, const float z)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, float, float, float)>(this, 11)(this, x, y, z);
		}

	public:
		void SetFloat(const float val)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, float)>(this, 4)(this, val);
		}

		void SetInt(const int val)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, int)>(this, 5)(this, val);
		}

		void SetString(char const* val)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, char const*)>(this, 6)(this, val);
		}

		void SetMatrix(matrix3x4_t& matrix)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, matrix3x4_t&)>(this, 6)(this, matrix);
		}

		void SetVectorComponent(const float val, const int comp)
		{
			GetFunctionTARDasshloe<void(__thiscall*)(void*, float, int)>(this, 26)(this, val, comp);
		}

		void SetVector(const Vector2D vector)
		{
			SetVectorInternal(vector.x, vector.y);
		}

		void SetVector(const Vector vector)
		{
			SetVectorInternal(vector.x, vector.y, vector.z);
		}
	};
}