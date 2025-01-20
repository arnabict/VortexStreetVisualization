#include "FeatureFlow.hpp"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <Eigen/Dense>
#include "AmiraReader.hpp"
#include "AmiraWriter.hpp"

namespace vispro
{
    void FeatureFlow::Compute(const char* velocityPathPrev, const char* velocityPathCurr, const char* velocityPathNext, int deltaSteps, const char* featureFlowPath) {
        vtkSmartPointer<vtkImageData> velocityImagePrev =
            AmiraReader::ReadField(velocityPathPrev, "velocity");
        vtkSmartPointer<vtkImageData> velocityImageCurr =
            AmiraReader::ReadField(velocityPathCurr, "velocity");
        vtkSmartPointer<vtkImageData> velocityImageNext =
            AmiraReader::ReadField(velocityPathNext, "velocity");

        if (!velocityImagePrev || !velocityImageCurr || !velocityImageNext) {
            throw std::runtime_error("Failed to read input velocity fields.");
        }

        vtkDataArray* velocityDataPrev = velocityImagePrev->GetPointData()->GetVectors();
        vtkDataArray* velocityDataCurr = velocityImageCurr->GetPointData()->GetVectors();
        vtkDataArray* velocityDataNext = velocityImageNext->GetPointData()->GetVectors();

        vtkFloatArray* velocityArrayPrev = vtkFloatArray::SafeDownCast(velocityDataPrev);
        vtkFloatArray* velocityArrayCurr = vtkFloatArray::SafeDownCast(velocityDataCurr);
        vtkFloatArray* velocityArrayNext = vtkFloatArray::SafeDownCast(velocityDataNext);

        /*vtkFloatArray* velocityArrayPrev =
            dynamic_cast<vtkFloatArray*>(velocityImagePrev->GetPointData()->GetArray("velocity"));
        vtkFloatArray* velocityArrayCurr =
            dynamic_cast<vtkFloatArray*>(velocityImageCurr->GetPointData()->GetArray("velocity"));
        vtkFloatArray* velocityArrayNext =
            dynamic_cast<vtkFloatArray*>(velocityImageNext->GetPointData()->GetArray("velocity"));*/

        if (!velocityArrayPrev || !velocityArrayCurr || !velocityArrayNext) {
            throw std::runtime_error("Failed to retrieve velocity arrays.");
        }
 
        vtkSmartPointer<vtkImageData> featureFlowImage = vtkSmartPointer<vtkImageData>::New();
        featureFlowImage->SetDimensions(velocityImageCurr->GetDimensions());
        featureFlowImage->SetOrigin(velocityImageCurr->GetOrigin());
        featureFlowImage->SetSpacing(velocityImageCurr->GetSpacing());

        /*int64_t numPoints = (int64_t)featureFlowImage->GetDimensions()[0] 
            * featureFlowImage->GetDimensions()[1] 
            * featureFlowImage->GetDimensions()[2];*/

        int* res = featureFlowImage->GetDimensions();
        int64_t numPoints = static_cast<int64_t>(res[0] * res[1] * res[2]);

        vtkSmartPointer<vtkFloatArray> featureFlowArrayU = vtkSmartPointer<vtkFloatArray>::New();
        vtkSmartPointer<vtkFloatArray> featureFlowArrayV = vtkSmartPointer<vtkFloatArray>::New();
        vtkSmartPointer<vtkFloatArray> featureFlowArrayW = vtkSmartPointer<vtkFloatArray>::New();

        featureFlowArrayU->SetNumberOfComponents(1);
        featureFlowArrayV->SetNumberOfComponents(1);
        featureFlowArrayW->SetNumberOfComponents(1);

        featureFlowArrayU->SetNumberOfTuples(numPoints);
        featureFlowArrayV->SetNumberOfTuples(numPoints);
        featureFlowArrayW->SetNumberOfTuples(numPoints);

        featureFlowArrayU->SetName("feature_flowU");
        featureFlowArrayV->SetName("feature_flowV");
        featureFlowArrayW->SetName("feature_flowW");

        featureFlowImage->GetPointData()->AddArray(featureFlowArrayU);
        featureFlowImage->GetPointData()->AddArray(featureFlowArrayV);
        featureFlowImage->GetPointData()->AddArray(featureFlowArrayW);

        /*featureFlowArray->SetNumberOfComponents(velocityDataCurr->GetNumberOfComponents());
        featureFlowArray->SetNumberOfTuples(velocityDataCurr->GetNumberOfTuples());
        featureFlowArray->SetName("feature_flow");*/
        //featureFlowImage->GetPointData()->AddArray(featureFlowArray);

        if (featureFlowArrayU->GetNumberOfTuples() != numPoints) {
            std::cerr << "Expected: " << numPoints << "\nFound: " << featureFlowArrayU->GetNumberOfTuples();
            return;
        }
        else
        {
            for (int iz = 0; iz < res[2]; ++iz) {
                for (int iy = 0; iy < res[1]; ++iy) {
                    for (int ix = 0; ix < res[0]; ++ix) {
                        int linear = (iz * res[1] + iy) * res[0] + ix;

                        if (linear < 0 || linear >= numPoints) {
                            throw std::runtime_error("Linear index out of bounds.");
                        }

                        int ix0 = std::max(0, ix - 1);
                        int ix1 = std::min(ix + 1, res[0] - 1);
                        int iy0 = std::max(0, iy - 1);
                        int iy1 = std::min(iy + 1, res[1] - 1);
                        int iz0 = std::max(0, iz - 1);
                        int iz1 = std::min(iz + 1, res[2] - 1);

                        int linear_x0 = (iz * res[1] + iy) * res[0] + ix0;
                        int linear_x1 = (iz * res[1] + iy) * res[0] + ix1;
                        int linear_y0 = (iz * res[1] + iy0) * res[0] + ix;
                        int linear_y1 = (iz * res[1] + iy1) * res[0] + ix;
                        int linear_z0 = (iz0 * res[1] + iy) * res[0] + ix;
                        int linear_z1 = (iz1 * res[1] + iy) * res[0] + ix;

                        Eigen::Vector3d vel_x0(velocityArrayCurr->GetTuple3(linear_x0));
                        Eigen::Vector3d vel_x1(velocityArrayCurr->GetTuple3(linear_x1));
                        Eigen::Vector3d vel_y0(velocityArrayCurr->GetTuple3(linear_y0));
                        Eigen::Vector3d vel_y1(velocityArrayCurr->GetTuple3(linear_y1));
                        Eigen::Vector3d vel_z0(velocityArrayCurr->GetTuple3(linear_z0));
                        Eigen::Vector3d vel_z1(velocityArrayCurr->GetTuple3(linear_z1));
                        Eigen::Vector3d vel_t0(velocityArrayPrev->GetTuple3(linear));
                        Eigen::Vector3d vel_t1(velocityArrayNext->GetTuple3(linear));

                        double spacing_x = (ix1 - ix0) * velocityImageCurr->GetSpacing()[0];
                        double spacing_y = (iy1 - iy0) * velocityImageCurr->GetSpacing()[1];
                        double spacing_z = (iz1 - iz0) * velocityImageCurr->GetSpacing()[2];
                        double spacing_t = deltaSteps * (2. / 150.);

                        Eigen::Vector3d dv_dx = (vel_x1 - vel_x0) / spacing_x;
                        Eigen::Vector3d dv_dy = (vel_y1 - vel_y0) / spacing_y;
                        Eigen::Vector3d dv_dz = (vel_z1 - vel_z0) / spacing_z;
                        Eigen::Vector3d dv_dt = (vel_t1 - vel_t0) / spacing_t;

                        Eigen::Matrix3d J;
                        J.col(0) = dv_dx;
                        J.col(1) = dv_dy;
                        J.col(2) = dv_dz;

                        Eigen::Matrix3d J_inv;
                        if (J.determinant() != 0) {
                            J_inv = J.inverse();
                        }
                        else {
                            J_inv = Eigen::Matrix3d::Zero();
                        }

                        Eigen::Vector3d featureFlow = -J_inv * dv_dt;

                        featureFlowArrayU->SetValue(linear,
                            featureFlow[0]);
                        featureFlowArrayV->SetValue(linear,
                            featureFlow[1]);
                        featureFlowArrayW->SetValue(linear,
                            featureFlow[2]);
                    }
                }
            }
        }
        
        AmiraWriter::WriteVectorField(featureFlowPath, "feature_flowU", "feature_flowV", "feature_flowW", featureFlowImage);
    }
}
