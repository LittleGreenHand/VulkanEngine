#pragma once

//不同类型的描述符的set编号
enum LayoutBindIndex {
	LBI_GLOBAL = 0,
	LBI_IBL,
	LBI_LIGHTS,
	LBI_MATERIALS,
	LBI_CUSTOM,
	LBI_COUNT
};

enum GLTFModels {
	M_Cube,
	M_Cerberus,
	M_Sponza,
	M_Sphere,
	M_Axis
};

//RenderPass索引
enum RenderPasses {
	RP_Light = 0,	//用于生成ShadowMap
	RP_Count
};

//Pipeline索引
enum Pipelines {
	PL_Skybox = 0,
	PL_PBR,
	PL_Count
};

