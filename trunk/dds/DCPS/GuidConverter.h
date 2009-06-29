/*
 * $Id$
 */

#ifndef DCPS_GUIDCONVERTER_H
#define DCPS_GUIDCONVERTER_H

#include <ostream>
#include <string>

#include "tao/Basic_Types.h"

#include "dds/DdsDcpsGuidC.h"

#include "GuidUtils.h"

#include "dcps_export.h"

namespace OpenDDS
{
namespace DCPS
{
/**
 * @class GuidConverter
 *
 * @brief Conversion processing and value testing utilities for RTPS
 *        GUID_t types.
 *
 * This class encapsulates the conversion of a GUID_t value to and from
 * other types used within OpenDDS.  This class handles opaque GUID_t values
 * only.  For specific access, the appropriate subclass must instead be used.
 *
 * Since the GUID_t type is formed of octets in network order, we do all
 * processing byte by byte to avoid any endian issues.
 *
 * Currently the GUID_t is mapped from various internal values.
 * These mappings are:
 *
 * byte  structure reference     content
 * ---- ---------------------    --------------------------
 *   0  GUID_t.guidPrefix[ 0] == VendorId_t == 0x00 for OCI (used for OpenDDS)
 *   1  GUID_t.guidPrefix[ 1] == VendorId_t == 0x03 for OCI (used for OpenDDS)
 *   2  GUID_t.guidPrefix[ 2] == 0x00
 *   3  GUID_t.guidPrefix[ 3] == 0x00
 *
 *   4  GUID_t.guidPrefix[ 4] == <private> (MS byte)
 *   5  GUID_t.guidPrefix[ 5] == <private>
 *   6  GUID_t.guidPrefix[ 6] == <private>
 *   7  GUID_t.guidPrefix[ 7] == <private> (LS byte)
 *
 *   8  GUID_t.guidPrefix[ 8] == <private> (MS byte)
 *   9  GUID_t.guidPrefix[ 9] == <private>
 *  10  GUID_t.guidPrefix[10] == <private>
 *  11  GUID_t.guidPrefix[11] == <private> (LS byte)
 *
 *  12  GUID_t.entityId.entityKey[ 0] == entity id[0] (MS byte)
 *  13  GUID_t.entityId.entityKey[ 1] == entity id[1]
 *  14  GUID_t.entityId.entityKey[ 2] == entity id[2] (LS byte)
 *  15  GUID_t.entityId.entityKind    == entity kind
 */
class OpenDDS_Dcps_Export GuidConverter
{
public:
  /// Construct from a GUID_t.
  GuidConverter(const GUID_t& guid);

  /// Virtual destructor (this is a base class).
  virtual ~GuidConverter();

  /// Calculate the CRC32 checksum.
  long checksum() const;

  /// Extract the VendorId value.
  long vendorId() const;

  /// Extract the EntityId value.
  long entityId() const;

  /// Extract the EntityKey value.
  long entityKey() const;

  /// Extract the EntityKind value.
  EntityKind entityKind() const;

  /// Convert to diagnostic string.
  operator std::string() const;
#ifdef ACE_HAS_WCHAR
  operator std::wstring() const;
#endif

protected:
  const GUID_t guid_;
};

} // namespace
} // namespace

OpenDDS_Dcps_Export std::ostream&
operator<<(std::ostream& os, const OpenDDS::DCPS::GuidConverter& rhs);

#ifdef ACE_HAS_WCHAR
OpenDDS_Dcps_Export std::wostream&
operator<<(std::wostream& os, const OpenDDS::DCPS::GuidConverter& rhs);
#endif


#endif /* DCPS_GUIDCONVERTER_H */
