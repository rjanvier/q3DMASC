//##########################################################################
//#                                                                        #
//#                     CLOUDCOMPARE PLUGIN: q3DMASC                       #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 or later of the License.      #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                 COPYRIGHT: Dimitri Lague / CNRS / UEB                  #
//#                                                                        #
//##########################################################################

#include "Features.h"

//Local
#include "ScalarFieldWrappers.h"
#include "q3DMASCTools.h"

//qCC_io
#include <LASFields.h>

//qCC_db
#include <ccPointCloud.h>
#include <ccScalarField.h>

//system
#include <assert.h>

static const char* s_echoRatioSFName = "EchoRat";
static const char* s_NIRSFName = "NIR";
static const char* s_M3C2SFName = "M3C2 distance";
static const char* s_PCVSFName = "Illuminance (PCV)";
static const char* s_normDipSFName = "Norm dip";
static const char* s_normDipDirSFName = "Norm dip dir.";

using namespace masc;

QSharedPointer<IScalarFieldWrapper> PointFeature::retrieveField(ccPointCloud* cloud, QString& error)
{
	if (!cloud)
	{
		assert(false);
		return QSharedPointer<IScalarFieldWrapper>(nullptr);
	}
	
	switch (type)
	{
	case PointFeature::Intensity:
	{
		CCLib::ScalarField* sf = Tools::RetrieveSF(cloud, LAS_FIELD_NAMES[LAS_INTENSITY], false);
		if (!sf)
		{
			error = "Cloud has no 'intensity' scalar field";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(sf));
	}
	case PointFeature::X:
		return QSharedPointer<IScalarFieldWrapper>(new DimScalarFieldWrapper(cloud, DimScalarFieldWrapper::DimX));
	case PointFeature::Y:
		return QSharedPointer<IScalarFieldWrapper>(new DimScalarFieldWrapper(cloud, DimScalarFieldWrapper::DimY));
	case PointFeature::Z:
		return QSharedPointer<IScalarFieldWrapper>(new DimScalarFieldWrapper(cloud, DimScalarFieldWrapper::DimZ));
	case PointFeature::NbRet:
	{
		CCLib::ScalarField* sf = Tools::RetrieveSF(cloud, LAS_FIELD_NAMES[LAS_NUMBER_OF_RETURNS], false);
		if (!sf)
		{
			error = "Cloud has no 'number of returns' scalar field";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(sf));
	}
	case PointFeature::RetNb:
	{
		CCLib::ScalarField* sf = Tools::RetrieveSF(cloud, LAS_FIELD_NAMES[LAS_RETURN_NUMBER], false);
		if (!sf)
		{
			error = "Cloud has no 'return number' scalar field";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(sf));
	}
	case PointFeature::EchoRat:
	{
		//retrieve the two scalar fields 'p/q'
		CCLib::ScalarField* numberOfRetSF = Tools::RetrieveSF(cloud, LAS_FIELD_NAMES[LAS_NUMBER_OF_RETURNS], false);
		if (!numberOfRetSF)
		{
			error = "Can't compute the 'echo ratio' field: no 'Number of Return' SF available";
			return nullptr;
		}
		CCLib::ScalarField* retNumberSF = Tools::RetrieveSF(cloud, LAS_FIELD_NAMES[LAS_RETURN_NUMBER], false);
		if (!retNumberSF)
		{
			error = "Can't compute the 'echo ratio' field: no 'Return number' SF available";
			return nullptr;
		}
		if (retNumberSF->size() != numberOfRetSF->size() || retNumberSF->size() != cloud->size())
		{
			error = "Internal error (inconsistent scalar fields)";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldRatioWrapper(retNumberSF, numberOfRetSF, "EchoRat"));
	}
	case PointFeature::R:
		return QSharedPointer<IScalarFieldWrapper>(new ColorScalarFieldWrapper(cloud, ColorScalarFieldWrapper::Red));
	case PointFeature::G:
		return QSharedPointer<IScalarFieldWrapper>(new ColorScalarFieldWrapper(cloud, ColorScalarFieldWrapper::Green));
	case PointFeature::B:
		return QSharedPointer<IScalarFieldWrapper>(new ColorScalarFieldWrapper(cloud, ColorScalarFieldWrapper::Blue));
	case PointFeature::NIR:
	{
		CCLib::ScalarField* sf = Tools::RetrieveSF(cloud, s_NIRSFName, false);
		if (!sf)
		{
			error = "Cloud has no 'NIR' scalar field";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(sf));
	}
	case PointFeature::DipAng:
	case PointFeature::DipDir:
	{
		//we need normals to compute the dip and dip direction!
		if (!cloud->hasNormals())
		{
			error = "Cloud has no normals: can't compute dip or dip dir. angles";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new NormDipAndDipDirFieldWrapper(cloud, type == PointFeature::DipAng ? NormDipAndDipDirFieldWrapper::Dip : NormDipAndDipDirFieldWrapper::DipDir));
	}
	case PointFeature::M3C2:
	{
		CCLib::ScalarField* sf = Tools::RetrieveSF(cloud, s_M3C2SFName, true);
		if (!sf)
		{
			error = "Cloud has no 'm3c2 distance' scalar field";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(sf));
	}
	case PointFeature::PCV:
	{
		CCLib::ScalarField* sf = Tools::RetrieveSF(cloud, s_PCVSFName, true);
		if (!sf)
		{
			error = "Cloud has no 'PCV/Illuminance' scalar field";
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(sf));
	}
	case PointFeature::SF:
		if (sourceSFIndex < 0 || sourceSFIndex >= static_cast<int>(cloud->getNumberOfScalarFields()))
		{
			error = QString("Can't retrieve the specified SF: invalid index (%1)").arg(sourceSFIndex);
			return nullptr;
		}
		return QSharedPointer<IScalarFieldWrapper>(new ScalarFieldWrapper(cloud->getScalarField(sourceSFIndex)));
	default:
		break;
	}

	error = "Unhandled feature type";
	return nullptr;
}

static bool ExtractStatFromSF(	const CCVector3& queryPoint,
								const CCLib::DgmOctree* octree,
								unsigned char octreeLevel,
								Feature::Stat stat,
								const IScalarFieldWrapper& inputField,
								PointCoordinateType radius,
								double& outputValue)
{
	if (!octree)
	{
		assert(false);
		return false;
	}
	outputValue = std::numeric_limits<double>::quiet_NaN();

	//spherical neighborhood extraction structure
	CCLib::DgmOctree::NearestNeighboursSphericalSearchStruct nNSS;
	{
		nNSS.level = octreeLevel;
		nNSS.queryPoint = queryPoint;
		nNSS.prepare(radius, octree->getCellSize(nNSS.level));
		octree->getTheCellPosWhichIncludesThePoint(&nNSS.queryPoint, nNSS.cellPos, nNSS.level);
		octree->computeCellCenter(nNSS.cellPos, nNSS.level, nNSS.cellCenter);
	}

	//we extract the point's neighbors
	unsigned kNN = octree->findNeighborsInASphereStartingFromCell(nNSS, radius, true);
	if (kNN == 0)
	{
		return true;
	}

	//specific case
	if (stat == Feature::RANGE)
	{
		double minValue = 0;
		double maxValue = 0;

		for (unsigned k = 0; k < kNN; ++k)
		{
			unsigned index = nNSS.pointsInNeighbourhood[k].pointIndex;
			double v = inputField.pointValue(index);

			//track min and max values
			if (k != 0)
			{
				if (v < minValue)
					minValue = v;
				else if (v > maxValue)
					maxValue = v;
			}
			else
			{
				minValue = maxValue = v;
			}
		}

		outputValue = maxValue - minValue;
		return true;
	}

	bool withSums = (stat == Feature::MEAN || stat == Feature::STD || stat == Feature::SKEW);
	bool withMode = (stat == Feature::MODE || stat == Feature::SKEW);
	double sum = 0.0;
	double sum2 = 0.0;
	QMap<float, unsigned> modeCounter;

	for (unsigned k = 0; k < kNN; ++k)
	{
		unsigned index = nNSS.pointsInNeighbourhood[k].pointIndex;
		double v = inputField.pointValue(index);

		if (withSums)
		{
			//compute average and std. dev.
			sum += v;
			sum2 += v * v;
		}

		if (withMode)
		{
			//store the number of occurences of each value
			//DGM TODO: it would be better with a custom 'resolution' if the field is not an integer one
			float vf = static_cast<float>(v);
			if (modeCounter.contains(vf))
			{
				++modeCounter[vf];
			}
			else
			{
				modeCounter[vf] = 1;
			}
		}
	}

	double mode = std::numeric_limits<double>::quiet_NaN();
	if (withMode)
	{
		//look for the value with the highest frequency
		unsigned maxCounter = 0;
		for (QMap<ScalarType, unsigned>::const_iterator it = modeCounter.begin(); it != modeCounter.end(); ++it)
		{
			if (it.value() > maxCounter)
			{
				maxCounter = it.value();
				mode = it.key();
			}
		}
	}

	switch (stat)
	{
	case Feature::MEAN:
		outputValue = sum / kNN;
		break;
	case Feature::MODE:
		outputValue = mode;
		break;
	case Feature::STD:
		outputValue = sqrt(std::abs(sum2 * kNN - sum * sum)) / kNN;
		break;
	case Feature::RANGE:
		//we can't be here
		assert(false);
		return false;
	case Feature::SKEW:
	{
		double mean = sum / kNN;
		double std = sqrt(std::abs(sum2 / kNN - mean * mean));
		if (std > std::numeric_limits<float>::epsilon()) //arbitrary epsilon
		{
			outputValue = (mean - mode) / std;
		}
		break;
	}
	default:
		ccLog::Warning("Unhandled STAT measure");
		assert(false);
		return false;
	}

	return true;
}

static CCLib::ScalarField* ExtractStat(	const CorePoints& corePoints, 
										ccPointCloud* sourceCloud,
										const IScalarFieldWrapper* sourceField,
										double scale,
										Feature::Stat stat,
										const char* resultSFName,
										CCLib::GenericProgressCallback* progressCb = nullptr)
{
	if (!corePoints.cloud || !sourceCloud || !sourceField || scale <= 0.0 || stat == Feature::NO_STAT || !resultSFName)
	{
		//invalid input parameters
		assert(false);
		return nullptr;
	}

	ccOctree::Shared octree = sourceCloud->getOctree();
	if (!octree)
	{
		octree = sourceCloud->computeOctree(progressCb);
		if (!octree)
		{
			ccLog::Warning("Failed to compute octree");
			return nullptr;
		}
	}

	CCLib::ScalarField* resultSF = nullptr;
	int sfIdx = corePoints.cloud->getScalarFieldIndexByName(resultSFName);
	if (sfIdx >= 0)
	{
		resultSF = corePoints.cloud->getScalarField(sfIdx);
	}
	else
	{
		resultSF = new ccScalarField(resultSFName);
		if (!resultSF->resizeSafe(corePoints.cloud->size()))
		{
			ccLog::Warning("Not enough memory");
			resultSF->release();
			return nullptr;
		}
	}
	resultSF->fill(NAN_VALUE);

	PointCoordinateType radius = static_cast<PointCoordinateType>(scale / 2);
	unsigned char octreeLevel = octree->findBestLevelForAGivenNeighbourhoodSizeExtraction(radius); //scale is the diameter!

	unsigned pointCount = corePoints.size();
	progressCb->setInfo(qPrintable(QString("Computing field: %1\n(core points: %2)").arg(resultSFName).arg(pointCount)));
	CCLib::NormalizedProgress nProgress(progressCb, pointCount);

	for (unsigned i = 0; i < pointCount; ++i)
	{
		double outputValue = 0;
		if (!ExtractStatFromSF(	*corePoints.cloud->getPoint(i),
								octree.data(),
								octreeLevel,
								stat,
								*sourceField,
								radius,
								outputValue))
		{
			//unexpected error
			resultSF->release();
			return nullptr;
		}

		ScalarType v = static_cast<ScalarType>(outputValue);
		resultSF->setValue(i, v);

		if (progressCb && !nProgress.oneStep())
		{
			//process cancelled by the user
			ccLog::Warning("Process cancelled");
			resultSF->release();
			return nullptr;
		}
	}

	resultSF->computeMinAndMax();
	int newSFIdx = corePoints.cloud->addScalarField(static_cast<ccScalarField*>(resultSF));
	//update display
	if (corePoints.cloud->getDisplay())
	{
		corePoints.cloud->setCurrentDisplayedScalarField(newSFIdx);
		corePoints.cloud->getDisplay()->redraw();
	}

	return resultSF;
}

static bool PerformMathOp(CCLib::ScalarField* sf1, const CCLib::ScalarField* sf2, Feature::Operation op)
{
	if (!sf1 || !sf2 || sf1->size() != sf2->size() || op == Feature::NO_OPERATION)
	{
		//invalid input parameters
		return false;
	}

	for (unsigned i = 0; i < sf1->size(); ++i)
	{
		ScalarType s1 = sf1->getValue(i);
		ScalarType s2 = sf2->getValue(i);
		ScalarType s = NAN_VALUE;
		switch (op)
		{
		case Feature::MINUS:
			s = s1 - s2;
			break;
		case Feature::PLUS:
			s = s1 + s2;
			break;
		case Feature::DIVIDE:
			if (std::abs(s2) > std::numeric_limits<ScalarType>::epsilon())
				s = s1 / s2;
			break;
		case Feature::MULTIPLY:
			s = s1 * s2;
			break;
		default:
			assert(false);
			break;
		}
		sf1->setValue(i, s);
	}
	sf1->computeMinAndMax();

	return true;
}

bool PointFeature::prepare(	const CorePoints& corePoints,
							QString& error,
							CCLib::GenericProgressCallback* progressCb/*=nullptr*/)
{
	if (!cloud1 || !corePoints.cloud)
	{
		//invalid input
		assert(false);
		return false;
	}

	//look for the source field
	QSharedPointer<IScalarFieldWrapper> field1 = retrieveField(cloud1, error);
	if (!field1)
	{
		//error should be up to date
		return false;
	}

	//shall we extract a statistical measure? (= scaled feature)
	if (scaled())
	{
		if (stat == Feature::NO_STAT)
		{
			assert(false);
			ccLog::Warning("Scaled features (SCx) must have an associated STAT measure");
			return false;
		}

		QSharedPointer<IScalarFieldWrapper> field2;
		if (cloud2)
		{
			//no need to compute the second scalar field if no MATH operation has to be performed?!
			if (op != Feature::NO_OPERATION)
			{
				field2 = retrieveField(cloud2, error);
				if (!field2)
				{
					//error should be up to date
					return false;
				}
			}
			else
			{
				assert(false);
				ccLog::Warning("Feature has a second cloud associated but no MATH operation is defined");
			}
		}

		//build the final SF name
		QString resultSFName = cloud1Label + "." + field1->getName() + QString("_") + Feature::StatToString(stat);
		if (field2 && op != Feature::NO_OPERATION)
		{
			//include the math operation as well if necessary!
			resultSFName += "_" + Feature::OpToString(op) + "_" + cloud2Label + "." + field2->getName() + QString("_") + Feature::StatToString(stat);
		}
		resultSFName += "@" + QString::number(scale);

		CCLib::ScalarField* statSF1 = ExtractStat(corePoints, cloud1, field1.data(), scale, stat, qPrintable(resultSFName), progressCb);
		if (!statSF1)
		{
			error = QString("Failed to extract stat. from field '%1' @ scale %2").arg(field1->getName()).arg(scale);
			return false;
		}
		sourceName = statSF1->getName();

		if (cloud2 && field2 && op != Feature::NO_OPERATION)
		{
			QString resultSFName2 = cloud2Label + "." + field2->getName() + QString("_") + Feature::StatToString(stat) + "@" + QString::number(scale);
			int sfIndex2 = corePoints.cloud->getScalarFieldIndexByName(qPrintable(resultSFName2));
			CCLib::ScalarField* statSF2 = ExtractStat(corePoints, cloud2, field2.data(), scale, stat, qPrintable(resultSFName2), progressCb);
			if (!statSF2)
			{
				error = QString("Failed to extract stat. from field '%1' @ scale %2").arg(field2->getName()).arg(scale);
				return false;
			}

			//now perform the math operation
			if (!PerformMathOp(statSF1, statSF2, op))
			{
				error = "Failed to perform the MATH operation";
				return false;
			}

			if (sfIndex2 < 0)
			{
				//release some memory
				sfIndex2 = corePoints.cloud->getScalarFieldIndexByName(qPrintable(resultSFName2));
				corePoints.cloud->deleteScalarField(sfIndex2);
			}
		}

		return true;
	}
	else //non scaled feature
	{
		if (cloud1 != corePoints.cloud && cloud1 != corePoints.origin)
		{
			assert(false);
			error = "Scale-less features (SC0) can only be defined on the core points (origin) cloud";
			return false;
		}

		if (cloud2)
		{
			if (op != Feature::NO_OPERATION)
			{
				assert(false);
				ccLog::Warning("MATH operations cannot be performed on scale-less features (SC0)");
				return false;
			}
			else
			{
				assert(false);
				ccLog::Warning("Feature has a second cloud associated but no MATH operation is defined");
			}
		}

		//build the final SF name
		QString resultSFName = /*cloud1Label + "." + */field1->getName();
		//if (cloud2 && field2 && op != Feature::NO_OPERATION)
		//{
		//	resultSFName += QString("_") + Feature::OpToString(op) + "_" + field2->getName();
		//}

		//retrieve/create a SF to host the result
		CCLib::ScalarField* resultSF = nullptr;
		int sfIdx = corePoints.cloud->getScalarFieldIndexByName(qPrintable(resultSFName));
		if (sfIdx >= 0)
		{
			//reuse the existing field
			resultSF = corePoints.cloud->getScalarField(sfIdx);
		}
		else
		{
			//copy the SF1 field
			resultSF = new ccScalarField(qPrintable(resultSFName));
			if (!resultSF->resizeSafe(corePoints.cloud->size()))
			{
				error = "Not enough memory";
				resultSF->release();
				return false;
			}

			//copy the values
			for (unsigned i = 0; i < corePoints.size(); ++i)
			{
				resultSF->setValue(i, field1->pointValue(corePoints.originIndex(i)));
			}
			resultSF->computeMinAndMax();
			int newSFIdx = corePoints.cloud->addScalarField(static_cast<ccScalarField*>(resultSF));
			//update display
			if (corePoints.cloud->getDisplay())
			{
				corePoints.cloud->setCurrentDisplayedScalarField(newSFIdx);
				corePoints.cloud->getDisplay()->redraw();
			}
		}

		sourceName = resultSF->getName();

		//if (cloud2 && field2 && op != Feature::NO_OPERATION)
		//{
		//	//now perform the math operation
		//	if (!PerformMathOp(*field1, *field2, op, resultSF))
		//	{
		//		error = "Failed to perform the MATH operation";
		//		return false;
		//	}

		//	//sf2 is held by the second cloud for now
		//	//sf2->release();
		//	//sf2 = nullptr;
		//}

		return true;
	}
}

bool NeighborhoodFeature::prepare(	const CorePoints& corePoints,
									QString& error,
									CCLib::GenericProgressCallback* progressCb/*=nullptr*/)
{
	//TODO
	return false;
}

bool ContextBasedFeature::prepare(	const CorePoints& corePoints,
									QString& error,
									CCLib::GenericProgressCallback* progressCb/*=nullptr*/)
{
	//TODO
	return false;
}

bool DualCloudFeature::prepare(	const CorePoints& corePoints,
								QString& error,
								CCLib::GenericProgressCallback* progressCb/*=nullptr*/)
{
	//TODO
	return false;
}