#ifndef MTK_EIS_CALIBRATION_DATA_H
#define MTK_EIS_CALIBRATION_DATA_H


// Front cam
const int FRONTCAM_calibration_source = 3; // 0: Actually Calibrated, 1: Legacy, 2: From FOV, 3: From default(Rectilinear)
const int FRONTCAM_calibration_Domain_Width = 0;
const int FRONTCAM_calibration_Domain_Height = 0;
const int FRONTCAM_Active_Pixel_Width = 0;
const int FRONTCAM_Active_Pixel_Height = 0;
const int FRONTCAM_mesh_Width = 33;
const int FRONTCAM_mesh_Height = 19;
const int FRONTCAM_num_AfLensPosition = 1;
const double FRONTCAM_normalized_AfLensPositions[] = {0};
const int FRONTCAM_num_OpticalZoomPosition = 1;
const double FRONTCAM_normalized_OpticalZoomPositions[] = {0};
const double* FRONTCAM_meshVertXY = NULL;
const double* FRONTCAM_meshVertLngLat = NULL;
const int* FRONTCAM_meshTriangleVertIndices = NULL;


// Main cam
const int MAINCAM_calibration_source = 3;
const int MAINCAM_calibration_Domain_Width = 0;
const int MAINCAM_calibration_Domain_Height = 0;
const int MAINCAM_Active_Pixel_Width = 0;
const int MAINCAM_Active_Pixel_Height = 0;
const int MAINCAM_mesh_Width = 33;
const int MAINCAM_mesh_Height = 19;
const int MAINCAM_num_AfLensPosition = 1;
const double MAINCAM_normalized_AfLensPositions[] = {0};
const int MAINCAM_num_OpticalZoomPosition = 1;
const double MAINCAM_normalized_OpticalZoomPositions[] = {0};
const double* MAINCAM_meshVertXY = NULL;
const double* MAINCAM_meshVertLngLat = NULL;
const int* MAINCAM_meshTriangleVertIndices = NULL;


// Ultrawide cam
const int ULTRAWIDECAM_calibration_source = 3;
const int ULTRAWIDECAM_calibration_Domain_Width = 0;
const int ULTRAWIDECAM_calibration_Domain_Height = 0;
const int ULTRAWIDECAM_Active_Pixel_Width = 0;
const int ULTRAWIDECAM_Active_Pixel_Height = 0;
const int ULTRAWIDECAM_mesh_Width = 33;
const int ULTRAWIDECAM_mesh_Height = 19;
const int ULTRAWIDECAM_num_AfLensPosition = 1;
const double ULTRAWIDECAM_normalized_AfLensPositions[] = {0};
const int ULTRAWIDECAM_num_OpticalZoomPosition = 1;
const double ULTRAWIDECAM_normalized_OpticalZoomPositions[] = {0};
const double* ULTRAWIDECAM_meshVertXY = NULL;
const double* ULTRAWIDECAM_meshVertLngLat = NULL;
const int* ULTRAWIDECAM_meshTriangleVertIndices = NULL;


// Tele cam
const int TELECAM_calibration_source = 3;
const int TELECAM_calibration_Domain_Width = 0;
const int TELECAM_calibration_Domain_Height = 0;
const int TELECAM_Active_Pixel_Width = 0;
const int TELECAM_Active_Pixel_Height = 0;
const int TELECAM_mesh_Width = 33;
const int TELECAM_mesh_Height = 19;
const int TELECAM_num_AfLensPosition = 1;
const double TELECAM_normalized_AfLensPositions[] = {0};
const int TELECAM_num_OpticalZoomPosition = 1;
const double TELECAM_normalized_OpticalZoomPositions[] = {0};
const double* TELECAM_meshVertXY = NULL;
const double* TELECAM_meshVertLngLat = NULL;
const int* TELECAM_meshTriangleVertIndices = NULL;

// Default
const int DEFAULT_calibration_source = 3; // 0: Actually Calibrated, 1: Legacy, 2: From FOV, 3: From default(Rectilinear)
const int DEFAULT_calibration_Domain_Width = 0;
const int DEFAULT_calibration_Domain_Height = 0;
const int DEFAULT_mesh_Width = 33;
const int DEFAULT_mesh_Height = 19;
const int DEFAULT_num_AfLensPosition = 1;
const double DEFAULT_normalized_AfLensPositions[] = {0};
const int DEFAULT_num_OpticalZoomPosition = 1;
const double DEFAULT_normalized_OpticalZoomPositions[] = {0};
const double* DEFAULT_meshVertXY = NULL;
const double* DEFAULT_meshVertLngLat = NULL;
const int* DEFAULT_meshTriangleVertIndices = NULL;

#endif

