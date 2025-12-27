#define STATIC_KEY_DIM_NUM 12
#define DYNAMIC_KEY_DIM_NUM 9

#define SCENARIO_TREE_DIM_SQL "Project, Action, Standard, SensorMode, Flash, YUVSize, Zoom, App, SensorFeature, CustomFeature, Feature, Custom, "

typedef enum
{
    EDim_STATIC_START,
    EDim_Project = EDim_STATIC_START,
    EDim_Action,
    EDim_Standard,
    EDim_SensorMode,
    EDim_Flash,
    EDim_YUVSize,
    EDim_Zoom,
    EDim_App,
    EDim_SensorFeature,
    EDim_CustomFeature,
    EDim_Feature,
    EDim_Custom,
    EDim_STATIC_END = EDim_Custom,
    EDim_DYNAMIC_START,
    EDim_ToneGain = EDim_DYNAMIC_START,
    EDim_DR,
    EDim_Tripod,
    EDim_Stage,
    EDim_FaceDetection,
    EDim_Ratio,
    EDim_LV,
    EDim_CT,
    EDim_ISO,
    EDim_DYNAMIC_END = EDim_ISO,
    EDim_NUM,
} EDim_T;

