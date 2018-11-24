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
#include "FeaturesInterface.h"

namespace masc
{
	//! Dual-cloud feature
	struct DualCloudFeature : public Feature
	{
	public: //DualCloudFeatureType

		enum DualCloudFeatureType
		{
			Invalid = 0
			, IDIFF
		};

		static QString ToString(DualCloudFeatureType type)
		{
			switch (type)
			{
			case Invalid:
				return "Invalid";
			case IDIFF:
				return "IDIFF";
			default:
				assert(false);
				break;
			}
			return "Invalid";
		}

		static inline DualCloudFeatureType FromString(const QString& token) { return FromUpperString(token.toUpper()); }
		static DualCloudFeatureType FromUpperString(const QString& token)
		{
			if (token == "IDIFF")
				return IDIFF;

			assert(false);
			return Invalid;
		}

	public: //methods

		//! Default constructor
		DualCloudFeature(DualCloudFeatureType p_type)
			: type(p_type)
		{}

		//! Returns the feature type
		virtual Type getType() const override { return Type::DualCloudFeature; }
		//! Clones this feature
		virtual Feature::Shared clone() const override { return Feature::Shared(new DualCloudFeature(*this)); }
		//! Prepares the feature (compute the scalar field, etc.)
		virtual bool prepare(const CorePoints& corePoints, QString& error, CCLib::GenericProgressCallback* progressCb = nullptr) override;
		//! Returns the descriptor for this particular feature
		virtual QString toString() const override
		{
			//use the default keyword + "_SC" + the scale
			return ToString(type) + "_SC" + QString::number(scale);
		}

		//! Dual-cloud feature type
		/** \warning different from the feature type
		**/
		DualCloudFeatureType type;
	};
}