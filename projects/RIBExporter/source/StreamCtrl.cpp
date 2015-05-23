/**
 * 情報保存用.
 */

#include "StreamCtrl.h"

/**
 * RIBエクスポートのダイアログパラメータをstreamに保存.
 */
void StreamCtrl::SaveRIBExportDlg (sxsdk::scene_interface* scene, const RIBExportData& data)
{
	try {
		compointer<sxsdk::stream_interface> stream(scene->create_attribute_stream_interface_with_uuid(RIB_EXPORTER_ID));
		if (!stream) return;

		stream->set_pointer(0);
		stream->set_size(0);

		int version = RIB_EXPORT_DLG_VERSION;
		stream->write_int(version);

		int iDat;

		iDat = (int)data.type;
		stream->write_int(iDat);

		stream->write_float(data.pixelVariance);
		stream->write_float(data.minSamples);
		stream->write_float(data.maxSamples);

		iDat = data.incremental ? 1 : 0;
		stream->write_int(iDat);

		stream->write_float(data.bias);
		stream->write_int(data.maxDiffuseDepth);
		stream->write_int(data.maxSpecularDepth);
		stream->write_int((int)data.renderingFormatType);
		stream->write_int((int)data.renderingImageType);

		iDat = data.outputBackgroundImage ? 1 : 0;
		stream->write_int(iDat);
		stream->write_int((int)data.backgroundImageSize);
		stream->write_float(data.backgroundImageIntensity);
		iDat = data.backgroundDraw ? 1 : 0;
		stream->write_int(iDat);

		iDat = data.colorColorToLinear ? 1 : 0;
		stream->write_int(iDat);
		iDat = data.colorTextureToLinear ? 1 : 0;
		stream->write_int(iDat);

		iDat = data.lightDayLight ? 1 : 0;
		stream->write_int(iDat);

		// ver.1.0.0.1 - .
		iDat = data.statisticsEndOfFrame ? 1 : 0;
		stream->write_int(iDat);

		iDat = data.statisticsXMLFile ? 1 : 0;
		stream->write_int(iDat);

	} catch (...) { }
}

/**
 * RIBエクスポートのダイアログパラメータをstreamから呼び出し.
 */
RIBExportData StreamCtrl::LoadRIBExportDlg (sxsdk::scene_interface* scene)
{
	RIBExportData data;
	data.Clear();

	try {
		compointer<sxsdk::stream_interface> stream(scene->get_attribute_stream_interface_with_uuid(RIB_EXPORTER_ID));
		if (!stream) return data;

		stream->set_pointer(0);

		int version = 0;
		stream->read_int(version);

		int iDat = 0;
		stream->read_int((int &)data.type);
		stream->read_float(data.pixelVariance);
		stream->read_float(data.minSamples);
		stream->read_float(data.maxSamples);

		stream->read_int(iDat);
		data.incremental = iDat ? true : false;

		stream->read_float(data.bias);
		stream->read_int(data.maxDiffuseDepth);
		stream->read_int(data.maxSpecularDepth);
		stream->read_int((int &)data.renderingFormatType);
		stream->read_int((int &)data.renderingImageType);

		stream->read_int(iDat);
		data.outputBackgroundImage = iDat ? true : false;
		stream->read_int((int &)data.backgroundImageSize);
		stream->read_float(data.backgroundImageIntensity);
		stream->read_int(iDat);
		data.backgroundDraw = iDat ? true : false;

		stream->read_int(iDat);
		data.colorColorToLinear = iDat ? true : false;
		stream->read_int(iDat);
		data.colorTextureToLinear = iDat ? true : false;

		stream->read_int(iDat);
		data.lightDayLight = iDat ? true : false;

		// ver.1.0.0.1 - .
		if (version >= RIB_EXPORT_DLG_VERSION) {
			stream->read_int(iDat);
			data.statisticsEndOfFrame = iDat ? true : false;

			stream->read_int(iDat);
			data.statisticsXMLFile = iDat ? true : false;
		}

	} catch (...) { }

	return data;
}

/**
 * RenderManのマテリアルパラメータを持つか.
 */
bool StreamCtrl::HasRIBMaterial (sxsdk::shape_class& shape)
{
	try {
		compointer<sxsdk::stream_interface> stream(shape.get_attribute_stream_interface_with_uuid(RIB_MATERIAL_ID));
		if (!stream) return false;

		return true;
	} catch (...) { }

	return false;
}

/**
 * 指定の形状（マスターサーフェス）にRenderManのパラメータを保存.
 */
void StreamCtrl::SaveRIBMaterial (sxsdk::shape_class& shape, const CRISMaterialInfo& data)
{
	try {
		compointer<sxsdk::stream_interface> stream(shape.create_attribute_stream_interface_with_uuid(RIB_MATERIAL_ID));
		if (!stream) return;

		stream->set_pointer(0);
		stream->set_size(0);

		int version = RIB_MATERIAL_VERSION;
		stream->write_int(version);

		int iDat;
		iDat = data.useCustom ? 1 : 0;
		stream->write_int(iDat);

		iDat = (int)data.type;
		stream->write_int(iDat);

		iDat = (int)data.useDepth ? 1 : 0;
		stream->write_int(iDat);

		iDat = (int)data.visibilityDiffuse ? 1 : 0;
		stream->write_int(iDat);

		iDat = (int)data.visibilitySpecular ? 1 : 0;
		stream->write_int(iDat);

		iDat = (int)data.visibilityIndirect ? 1 : 0;
		stream->write_int(iDat);

		iDat = (int)data.visibilityCamera ? 1 : 0;
		stream->write_int(iDat);

		stream->write_int(data.maxdiffusedepth);
		stream->write_int(data.maxspeculardepth);

		// CPxrMaterialDiffuse.
		stream->write_float(data.pxrDiffuse.diffuseColor.red);
		stream->write_float(data.pxrDiffuse.diffuseColor.green);
		stream->write_float(data.pxrDiffuse.diffuseColor.blue);
		stream->write_float(data.pxrDiffuse.transmissionColor.red);
		stream->write_float(data.pxrDiffuse.transmissionColor.green);
		stream->write_float(data.pxrDiffuse.transmissionColor.blue);
		stream->write_float(data.pxrDiffuse.presence);

		// CPxrMaterialDisney.
		stream->write_float(data.pxrDisney.baseColor.red);
		stream->write_float(data.pxrDisney.baseColor.green);
		stream->write_float(data.pxrDisney.baseColor.blue);
		stream->write_float(data.pxrDisney.emitColor.red);
		stream->write_float(data.pxrDisney.emitColor.green);
		stream->write_float(data.pxrDisney.emitColor.blue);
		stream->write_float(data.pxrDisney.subsurface);
		stream->write_float(data.pxrDisney.subsurfaceColor.red);
		stream->write_float(data.pxrDisney.subsurfaceColor.green);
		stream->write_float(data.pxrDisney.subsurfaceColor.blue);
		stream->write_float(data.pxrDisney.metallic);
		stream->write_float(data.pxrDisney.specular);
		stream->write_float(data.pxrDisney.specularTint);
		stream->write_float(data.pxrDisney.roughness);
		stream->write_float(data.pxrDisney.anisotropic);
		stream->write_float(data.pxrDisney.sheen);
		stream->write_float(data.pxrDisney.sheenTint);

		// CPxrMaterialGlass.
		stream->write_float(data.pxrGlass.ior);
		stream->write_float(data.pxrGlass.roughness);
		stream->write_float(data.pxrGlass.reflectionColor.red);
		stream->write_float(data.pxrGlass.reflectionColor.green);
		stream->write_float(data.pxrGlass.reflectionColor.blue);
		stream->write_float(data.pxrGlass.reflectionGain);
		stream->write_float(data.pxrGlass.transmissionColor.red);
		stream->write_float(data.pxrGlass.transmissionColor.green);
		stream->write_float(data.pxrGlass.transmissionColor.blue);
		stream->write_float(data.pxrGlass.transmissionGain);
		stream->write_float(data.pxrGlass.absorptionGain);
		stream->write_float(data.pxrGlass.absorptionColor.red);
		stream->write_float(data.pxrGlass.absorptionColor.green);
		stream->write_float(data.pxrGlass.absorptionColor.blue);

		// CPxrMaterialConstant.
		stream->write_float(data.pxrConstant.emitColor.red);
		stream->write_float(data.pxrConstant.emitColor.green);
		stream->write_float(data.pxrConstant.emitColor.blue);

		// ラベルの指定.
		if (!data.useCustom) {
			stream->set_label("");
		} else {
			compointer<sxsdk::scene_interface> scene(shape.get_scene_interface());

			std::string labelName = "";
			switch (data.type) {
			case RIBParam::pxrDiffuse:
				labelName = scene->gettext("ris_material_diffuse");
				break;

			case RIBParam::pxrDisney:
				labelName = scene->gettext("ris_material_disney");
				break;

			case RIBParam::pxrGlass:
				labelName = scene->gettext("ris_material_glass");
				break;

			case RIBParam::pxrConstant:
				labelName = scene->gettext("ris_material_constant");
				break;

			case RIBParam::pxrSkin:
				labelName = scene->gettext("ris_material_skin");
				break;

			case RIBParam::pxrVolume:
				labelName = scene->gettext("ris_material_volume");
				break;
			}

			const std::string str = std::string("[") + labelName + std::string("]");
			stream->set_label(str.c_str());
		}

	} catch (...) { }
}

/**
 * 指定の形状（マスターサーフェス）のRenderManのパラメータを取得.
 */
CRISMaterialInfo StreamCtrl::LoadRIBMaterial (sxsdk::shape_class& shape)
{
	CRISMaterialInfo data;

	try {
		compointer<sxsdk::stream_interface> stream(shape.get_attribute_stream_interface_with_uuid(RIB_MATERIAL_ID));
		if (!stream) return data;

		stream->set_pointer(0);

		int version = 0;
		stream->read_int(version);

		int iDat;

		stream->read_int(iDat);
		data.useCustom = iDat ? true : false;

		stream->read_int(iDat);
		data.type = (RIBParam::MATERIAL_TYPE)iDat;

		stream->read_int(iDat);
		data.useDepth = iDat ? true : false;

		stream->read_int(iDat);
		data.visibilityDiffuse = iDat ? true : false;

		stream->read_int(iDat);
		data.visibilitySpecular = iDat ? true : false;

		stream->read_int(iDat);
		data.visibilityIndirect = iDat ? true : false;

		stream->read_int(iDat);
		data.visibilityCamera = iDat ? true : false;

		stream->read_int(data.maxdiffusedepth);
		stream->read_int(data.maxspeculardepth);

		// CPxrMaterialDiffuse.
		stream->read_float(data.pxrDiffuse.diffuseColor.red);
		stream->read_float(data.pxrDiffuse.diffuseColor.green);
		stream->read_float(data.pxrDiffuse.diffuseColor.blue);
		stream->read_float(data.pxrDiffuse.transmissionColor.red);
		stream->read_float(data.pxrDiffuse.transmissionColor.green);
		stream->read_float(data.pxrDiffuse.transmissionColor.blue);
		stream->read_float(data.pxrDiffuse.presence);

		// CPxrMaterialDisney.
		stream->read_float(data.pxrDisney.baseColor.red);
		stream->read_float(data.pxrDisney.baseColor.green);
		stream->read_float(data.pxrDisney.baseColor.blue);
		stream->read_float(data.pxrDisney.emitColor.red);
		stream->read_float(data.pxrDisney.emitColor.green);
		stream->read_float(data.pxrDisney.emitColor.blue);
		stream->read_float(data.pxrDisney.subsurface);
		stream->read_float(data.pxrDisney.subsurfaceColor.red);
		stream->read_float(data.pxrDisney.subsurfaceColor.green);
		stream->read_float(data.pxrDisney.subsurfaceColor.blue);
		stream->read_float(data.pxrDisney.metallic);
		stream->read_float(data.pxrDisney.specular);
		stream->read_float(data.pxrDisney.specularTint);
		stream->read_float(data.pxrDisney.roughness);
		stream->read_float(data.pxrDisney.anisotropic);
		stream->read_float(data.pxrDisney.sheen);
		stream->read_float(data.pxrDisney.sheenTint);

		// CPxrMaterialGlass.
		stream->read_float(data.pxrGlass.ior);
		stream->read_float(data.pxrGlass.roughness);
		stream->read_float(data.pxrGlass.reflectionColor.red);
		stream->read_float(data.pxrGlass.reflectionColor.green);
		stream->read_float(data.pxrGlass.reflectionColor.blue);
		stream->read_float(data.pxrGlass.reflectionGain);
		stream->read_float(data.pxrGlass.transmissionColor.red);
		stream->read_float(data.pxrGlass.transmissionColor.green);
		stream->read_float(data.pxrGlass.transmissionColor.blue);
		stream->read_float(data.pxrGlass.transmissionGain);
		stream->read_float(data.pxrGlass.absorptionGain);
		stream->read_float(data.pxrGlass.absorptionColor.red);
		stream->read_float(data.pxrGlass.absorptionColor.green);
		stream->read_float(data.pxrGlass.absorptionColor.blue);

		// CPxrMaterialConstant.
		stream->read_float(data.pxrConstant.emitColor.red);
		stream->read_float(data.pxrConstant.emitColor.green);
		stream->read_float(data.pxrConstant.emitColor.blue);

	} catch (...) { }

	return data;
}


/**
 * RenderManの面光源のパラメータを持つか.
 */
bool StreamCtrl::HasRIBAreaLight (sxsdk::shape_class& shape)
{
	if (shape.get_type() != sxsdk::enums::line) return false;

	try {
		compointer<sxsdk::stream_interface> stream(shape.get_attribute_stream_interface_with_uuid(RIB_AREA_LIGHT_ID));
		if (!stream) return false;

		return true;
	} catch (...) { }

	return false;
}

/**
 * 指定の面光源にRenderManのパラメータを保存.
 */
void StreamCtrl::SaveRIBAreaLight (sxsdk::shape_class& shape, const CPxrAreaLight& data)
{
	try {
		compointer<sxsdk::stream_interface> stream(shape.create_attribute_stream_interface_with_uuid(RIB_AREA_LIGHT_ID));
		if (!stream) return;

		stream->set_pointer(0);
		stream->set_size(0);

		int version = RIB_AREA_LIGHT_VERSION;
		stream->write_int(version);

		stream->write_float(data.intensity);
		stream->write_float(data.lightColor.red);
		stream->write_float(data.lightColor.green);
		stream->write_float(data.lightColor.blue);
		stream->write_float(data.areaNormalize);
		stream->write_float(data.specAmount.red);
		stream->write_float(data.specAmount.green);
		stream->write_float(data.specAmount.blue);
		stream->write_float(data.diffAmount.red);
		stream->write_float(data.diffAmount.green);
		stream->write_float(data.diffAmount.blue);
		stream->write_float(data.coneAngle);
		stream->write_float(data.penumbraAngle);
		stream->write_float(data.penumbraExponent);
		stream->write_float(data.profileRange);
		stream->write_float(data.cosinePower);
		stream->write_float(data.angularVisibility);
		stream->write_float(data.shadowColor.red);
		stream->write_float(data.shadowColor.green);
		stream->write_float(data.shadowColor.blue);
		stream->write_float(data.traceShadows);
		stream->write_float(data.adaptiveShadows);

		stream->set_label("[PxrAreaLight]");

	} catch (...) { }
}

/**
 * 指定の面光源のRenderManのパラメータを取得.
 */
CPxrAreaLight StreamCtrl::LoadRIBAreaLight (sxsdk::shape_class& shape)
{
	CPxrAreaLight data;

	try {
		compointer<sxsdk::stream_interface> stream(shape.get_attribute_stream_interface_with_uuid(RIB_AREA_LIGHT_ID));
		if (!stream) return data;

		int version = 0;
		stream->read_int(version);

		stream->read_float(data.intensity);
		stream->read_float(data.lightColor.red);
		stream->read_float(data.lightColor.green);
		stream->read_float(data.lightColor.blue);
		stream->read_float(data.areaNormalize);
		stream->read_float(data.specAmount.red);
		stream->read_float(data.specAmount.green);
		stream->read_float(data.specAmount.blue);
		stream->read_float(data.diffAmount.red);
		stream->read_float(data.diffAmount.green);
		stream->read_float(data.diffAmount.blue);
		stream->read_float(data.coneAngle);
		stream->read_float(data.penumbraAngle);
		stream->read_float(data.penumbraExponent);
		stream->read_float(data.profileRange);
		stream->read_float(data.cosinePower);
		stream->read_float(data.angularVisibility);
		stream->read_float(data.shadowColor.red);
		stream->read_float(data.shadowColor.green);
		stream->read_float(data.shadowColor.blue);
		stream->read_float(data.traceShadows);
		stream->read_float(data.adaptiveShadows);

	} catch (...) { }

	return data;
}

