#include "VolrenShader.hpp"
#include "Data.hpp"
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageData.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <qwidget.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include "QDoubleSlider.hpp"
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>

namespace vispro
{
    VolrenShader::VolrenShader() :
        Component("VolrenShader"),
        mVolren(NULL), mColorTransferFunction(NULL), mOpacityTransferFunction(NULL),
        mShadowEnabled(1)
    {
        mColorRange[0] = 10;
        mColorRange[1] = 3;
        mOpacityRange[0] = 0;
        mOpacityRange[1] = 3;
        mExtinction = 10;
    }
    VolrenShader::~VolrenShader() {}

    void VolrenShader::CreateWidget(QWidget* widget)
    {
        // create slider for setting the value ranges
        QDoubleSlider* colorMinSlider = CreateDoubleSlider(0, 10, mColorRange[0], &VolrenShader::SetMinColorValue);
        QDoubleSlider* colorMaxSlider = CreateDoubleSlider(0, 10, mColorRange[1], &VolrenShader::SetMaxColorValue);
        QDoubleSlider* opacityMinSlider = CreateDoubleSlider(0, 10, mOpacityRange[0], &VolrenShader::SetMinOpacityValue);
        QDoubleSlider* opacityMaxSlider = CreateDoubleSlider(0, 10, mOpacityRange[1], &VolrenShader::SetMaxOpacityValue);
        QDoubleSlider* extinctionSlider = CreateDoubleSlider(0, 10, mExtinction, &VolrenShader::SetExtinctionValue);

        // create checkbox for enabling the shadow computation
        QCheckBox* shadowCheckBox = new QCheckBox;
        shadowCheckBox->setChecked(mShadowEnabled);
        connect(shadowCheckBox, &QCheckBox::stateChanged, this, &VolrenShader::SetShadowEnabledValue);

        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Color min:")), colorMinSlider);
        layout->addRow(new QLabel(tr("Color max:")), colorMaxSlider);
        layout->addRow(new QLabel(tr("Opacity min:")), opacityMinSlider);
        layout->addRow(new QLabel(tr("Opacity max:")), opacityMaxSlider);
        layout->addRow(new QLabel(tr("Extinction:")), extinctionSlider);
        layout->addRow(new QLabel(tr("Shadows:")), shadowCheckBox);
        widget->setLayout(layout);
    }

    QDoubleSlider* VolrenShader::CreateDoubleSlider(double minValue, double maxValue, double initialValue, void (VolrenShader::* fn)(double)) const {
        QDoubleSlider* slider = new QDoubleSlider;
        slider->setMinimum(minValue * slider->intScaleFactor());    // minimal value on slider
        slider->setMaximum(maxValue * slider->intScaleFactor());    // maximal value on slider
        slider->setDoubleValue(initialValue);
        connect(slider, &QDoubleSlider::doubleValueChanged, this, fn);
        return slider;
    }

    vtkSmartPointer<vtkProp> VolrenShader::CreateActor()
    {
        mVolren = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
        mColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
        mOpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

        SetColormapBlue2Red();

        // assign transfer function to volume properties
        vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
        volumeProperty->SetColor(mColorTransferFunction);
        volumeProperty->SetScalarOpacity(mOpacityTransferFunction);
        volumeProperty->SetInterpolationTypeToLinear();

        // create volume actor and assign mapper and properties
        vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
        volume->SetMapper(mVolren);
        volume->SetProperty(volumeProperty);

        // set a fragment shader code that is slightly adjusted from the official implementation:
        // https://github.com/Kitware/VTK/blob/master/Rendering/VolumeOpenGL2/shaders/raycasterfs.glsl
        volume->GetShaderProperty()->SetFragmentShaderCode(
            "//VTK::System::Dec\n"
            "in vec3 ip_textureCoords;\n"
            "in vec3 ip_vertexPos;\n"
            "vec4 g_fragColor = vec4(0.0);\n"
            "vec3 g_dirStep;\n"
            "vec4 g_srcColor;\n"
            "vec4 g_eyePosObj;\n"
            "bool g_skip;\n"
            "float g_currentT;\n"
            "float g_lengthStep;\n"
            "float g_terminatePointMax;\n"
            "vec3 g_rayOrigin;\n"
            "vec3 g_rayTermination;\n"
            "vec3 g_dataPos;\n"
            "vec3 g_terminatePos;\n"
            "//VTK::CustomUniforms::Dec\n"
            "//VTK::Output::Dec\n"
            "//VTK::Base::Dec\n"
            "//VTK::Termination::Dec\n"
            "//VTK::ComputeRayDirection::Dec\n"
            "\n"
            "uniform sampler2D in_opacityTransferFunc_0[1];\n"
            "float computeOpacity(float scalar) {\n"
            "   return 1 - exp(-texture2D(in_opacityTransferFunc_0[0], vec2(scalar, 0)).r * in_sampleDistance * in_extinctionScale);\n"
            "}\n"
            "uniform sampler2D in_colorTransferFunc_0[1];\n"
            "vec3 computeColor(float scalar) {\n"
            "   return texture2D(in_colorTransferFunc_0[0], vec2(scalar, 0)).xyz; \n"
            "}\n"
            "vec4 WindowToNDC(const float xCoord, const float yCoord, const float zCoord) {\n"
            "  vec4 NDCCoord = vec4(0.0, 0.0, 0.0, 1.0);\n"
            "  NDCCoord.x = (xCoord - in_windowLowerLeftCorner.x) * 2.0 * in_inverseWindowSize.x - 1.0;\n"
            "  NDCCoord.y = (yCoord - in_windowLowerLeftCorner.y) * 2.0 * in_inverseWindowSize.y - 1.0;\n"
            "  NDCCoord.z = (2.0 * zCoord - (gl_DepthRange.near + gl_DepthRange.far)) / gl_DepthRange.diff;\n"
            "  return NDCCoord;\n"
            "}\n"
            "void initializeRayCast() {\n"
            "  g_fragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
            "  //VTK::Base::Init\n"
            "  //VTK::Terminate::Init\n"
            "}\n"
            "//////////////////////////////////////////////////////////////////////////////\n"
            "float computeShadow(vec3 pos) {\n"
            "  float transmittance = 1.0;\n"
            "  vec3 rayDir = vec3(0.57735);\n"
            "  vec3 dirStep = (ip_inverseTextureDataAdjusted * vec4(rayDir, 0.0)).xyz * in_sampleDistance;\n"
            "  pos += 5 * dirStep;\n"
            "  while (true) {\n"
            "    pos += dirStep;\n"
            "    float scalar = texture3D(in_volume[0], pos).r * in_volume_scale[0].r + in_volume_bias[0].r;\n"
            "    transmittance *= 1 - computeOpacity(scalar);\n"
            "    if(any(greaterThan(max(dirStep, vec3(0.0)) * (pos - in_texMax[0]), vec3(0.0))) ||\n"
            "       any(greaterThan(min(dirStep, vec3(0.0)) * (pos - in_texMin[0]), vec3(0.0))) ||\n"
            "       transmittance < 0.01)"
            "    break;\n"
            "  }\n"
            "  return transmittance;\n"
            "}\n"
            "void main()\n"
            "{\n"
            "  initializeRayCast();\n"
            "  while (true)\n"
            "  {\n"    
            "    float scalar = texture3D(in_volume[0], g_dataPos).r * in_volume_scale[0].r + in_volume_bias[0].r;\n"
            "    g_srcColor.a = computeOpacity(scalar);\n"
            "    if (g_srcColor.a > 0.0)\n"
            "    {\n"
            "      g_srcColor.rgb = computeColor(scalar);\n"
            "      if (in_shadowEnabled == 1)\n"
            "        g_srcColor.rgb *= computeShadow(g_dataPos);\n"
            "      g_fragColor.rgb += g_fragColor.a * g_srcColor.rgb * g_srcColor.a;\n"
            "      g_fragColor.a *= 1 - g_srcColor.a;\n"
            "    }\n"
            "    g_dataPos += g_dirStep;\n"
            "    if(any(greaterThan(max(g_dirStep, vec3(0.0))*(g_dataPos - in_texMax[0]),vec3(0.0))) ||\n"
            "       any(greaterThan(min(g_dirStep, vec3(0.0))*(g_dataPos - in_texMin[0]),vec3(0.0))) ||\n"
            "       g_fragColor.a < 0.01 || g_currentT >= g_terminatePointMax)\n"
            "      break;\n"  
            "    ++g_currentT;\n"
            "  }\n"
            "  gl_FragData[0] = vec4(g_fragColor.rgb, 1 - g_fragColor.a);\n"
            "}\n"
            //"TRIGGER ERROR"
            "\n");
        volume->GetShaderProperty()->GetFragmentCustomUniforms()->SetUniformi("in_shadowEnabled", mShadowEnabled);
        volume->GetShaderProperty()->GetFragmentCustomUniforms()->SetUniformf("in_extinctionScale", mExtinction);
        return volume;
    }

    // Sets a default color map.
    void VolrenShader::SetColormapBlue2Red()
    {
        mColorTransferFunction->RemoveAllPoints();
        mOpacityTransferFunction->RemoveAllPoints();
        const int numPoints = 256;
        double prev_xc = std::numeric_limits<double>::max();
        double prev_xo = std::numeric_limits<double>::max();
        for (int i = 0; i < numPoints; ++i)
        {
            double t = i / (double)(numPoints - 1);
            double xc = mColorRange[0] + t * (mColorRange[1] - mColorRange[0]);
            if (std::abs(prev_xc - xc) > 1E-2) {
                double r = std::min(std::max(0.0, (((5.0048 * t + -8.0915) * t + 1.1657) * t + 1.4380) * t + 0.6639), 1.0);
                double g = std::min(std::max(0.0, (((7.4158 * t + -15.9415) * t + 7.4696) * t + 1.2767) * t + -0.0013), 1.0);
                double b = std::min(std::max(0.0, (((6.1246 * t + -16.2287) * t + 11.9910) * t + -1.4886) * t + 0.1685), 1.0);
                mColorTransferFunction->AddRGBPoint(xc, r, g, b);
                prev_xc = xc;
            }
            double xo = mOpacityRange[0] + t * (mOpacityRange[1] - mOpacityRange[0]);
            if (std::abs(prev_xo - xo) > 1E-2) {
                mOpacityTransferFunction->AddPoint(xo, t);
                prev_xo = xo;
            }
        }
        // make sure the last point has full exinction.
        mOpacityTransferFunction->AddPoint(prev_xo, 1);
    }

    void VolrenShader::SetData(Data* data)
    {
        vtkImageData* imageData = data->GetField(Data::EField::FTLE);
        if (imageData == nullptr) return;
        mVolren->SetInputData(imageData);
        mVolren->Update();
    }

    void VolrenShader::SetMinColorValue(double minValue) {
        mColorRange[0] = minValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }
    void VolrenShader::SetMaxColorValue(double maxValue) {
        mColorRange[1] = maxValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }

    void VolrenShader::SetMinOpacityValue(double minValue) {
        mOpacityRange[0] = minValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }
    void VolrenShader::SetMaxOpacityValue(double maxValue) {
        mOpacityRange[1] = maxValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }

    void VolrenShader::SetExtinctionValue(double scale) {
        mExtinction = scale;
        vtkVolume::SafeDownCast(mActor)->GetShaderProperty()->GetFragmentCustomUniforms()->SetUniformf("in_extinctionScale", mExtinction);
        emit RequestRender();
    }

    void VolrenShader::SetShadowEnabledValue(int state) {
        mShadowEnabled = state == 0 ? 0 : 1;
        vtkVolume::SafeDownCast(mActor)->GetShaderProperty()->GetFragmentCustomUniforms()->SetUniformi("in_shadowEnabled", mShadowEnabled);
        emit RequestRender();
    }
}