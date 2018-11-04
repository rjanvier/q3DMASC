#pragma once

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


//Local
#include "CorePoints.h"

//Qt
#include <QString>

//system
#include <assert.h>

class ccPointCloud;

namespace masc
{
	//! Generic feature descriptor
	struct Feature
	{
	public: //shortcuts

		//!Shared type
		typedef QSharedPointer<Feature> Shared;

		//! Set of features
		typedef std::vector<Shared> Set;

	public: //enumerators

		//! Feature type
		enum class Type
		{
			PointFeature,			/*!< Point features (scalar field, etc.) */
			NeighborhoodFeature,	/*!< Neighborhood based features for a given scale */
			ContextBasedFeature,	/*!< Contextual based features */
			DualCloudFeature,		/*!< Dual Cloud features: requires 2 point clouds */
			Invalid					/*!< Invalid feature */
		};

		enum Stat
		{
			NO_STAT,
			MEAN,
			MODE, //number with the highest frequency
			STD,
			RANGE,
			SKEW //(SKEW = (MEAN - MODE)/STD)
		};

		static QString StatToString(Stat stat)
		{
			switch (stat)
			{
			case MEAN:
				return "MEAN";
			case MODE:
				return "MODE";
			case STD:
				return "STD";
			case RANGE:
				return "RANGE";
			case SKEW:
				return "SKEW";
			default:
				break;
			};
			return QString();
		}

		enum Operation
		{
			NO_OPERATION, MINUS, PLUS, DIVIDE, MULTIPLY
		};

		static QString OpToString(Operation op)
		{
			switch (op)
			{
			case MINUS:
				return "MINUS";
			case PLUS:
				return "PLUS";
			case DIVIDE:
				return "DIVIDE";
			case MULTIPLY:
				return "MULTIPLY";
			default:
				break;
			};
			return QString();
		}

		//! Sources of values for this feature
		enum Source
		{
			ScalarField, DimX, DimY, DimZ, Red, Green, Blue
		};

	public: //methods

		//! Default constructor
		Feature(double p_scale = std::numeric_limits<double>::quiet_NaN(), Source p_source = ScalarField, QString p_sourceName = QString())
			: scale(p_scale)
			, cloud1(nullptr)
			, cloud2(nullptr)
			, source(p_source)
			, sourceName(p_sourceName)
			, stat(NO_STAT)
			, op(NO_OPERATION)
		{}

		//! Returns the type (must be reimplemented by child struct)
		virtual Type getType() const = 0;

		//! Returns the formatted description
		virtual QString toString() const = 0;

		//! Clones this feature
		virtual Feature::Shared clone() const = 0;

		//! Prepares the feature (compute the scalar field, etc.)
		virtual bool prepare(const CorePoints& corePoints, QString& error, CCLib::GenericProgressCallback* progressCb = nullptr) = 0;

		//! Returns whether the feature has an associated scale
		inline bool scaled() const { return std::isfinite(scale); }

		//! Checks the feature definition validity
		virtual bool checkValidity(QString &error) const
		{
			unsigned char cloudCount = (cloud1 ? (cloud2 ? 2 : 1) : 0);

			if (cloudCount == 0)
			{
				error = "feature has no associated cloud";
				return false;
			}

			if (scaled() && stat == NO_STAT)
			{
				error = "scaled features need a STAT measure to be defined";
				return false;
			}

			if (stat != NO_STAT)
			{
				if (getType() != Type::PointFeature)
				{
					error = "STAT measures can only be defined on Point features";
					return false;
				}
				if (!scaled())
				{
					error = "STAT measures need at least one scale to be defined";
					return false;
				}
			}

			if (op != NO_OPERATION)
			{
				if (!scaled())
				{
					error = "math operations can't be defined on scale-less features (SC0)";
					return false;
				}
				if (getType() == Type::DualCloudFeature)
				{
					error = "math operations can't be defined on dual-cloud features";
					return false;
				}
				if (cloudCount < 2)
				{
					error = "at least two clouds are required to apply math operations";
					return false;
				}
			}
			if (getType() == Feature::Type::DualCloudFeature || getType() == Feature::Type::ContextBasedFeature)
			{
				if (cloudCount < 2)
				{
					error = "at least two clouds are required to compute dual-cloud or context-based features";
					return false;
				}
			}

			return true;
		}

	public: //members

		//! Scale (diameter)
		double scale;

		ccPointCloud *cloud1, *cloud2;
		QString cloud1Label, cloud2Label;
	
		Source source; //values source
		QString sourceName; //feature source name (mandatory for scalar fields if the SF index is not set)
	
		Stat stat; //only considered if a scale is defined
		Operation op; //only considered if 2 clouds are defined
	};
}