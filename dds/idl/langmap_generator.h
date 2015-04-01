/*
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef langmap_generator_H
#define langmap_generator_H

#include "dds_generator.h"

/// Generate code for the IDL -> Programming Language mapping
/// For example, IDL structs -> C++ structs, etc.
/// Enabled by the -L* command line options
class langmap_generator : public dds_generator {

public:
  langmap_generator() {}
  void init();

private:
  bool gen_const(UTL_ScopedName* name, bool nestedInInteface,
                 AST_Constant* constant);

  bool gen_enum(UTL_ScopedName* name,
                const std::vector<AST_EnumVal*>& contents, const char* repoid);

  bool gen_struct(UTL_ScopedName* name,
                  const std::vector<AST_Field*>& fields,
                  AST_Type::SIZE_TYPE size, const char* repoid);

  bool gen_struct_fwd(UTL_ScopedName* name,
                      AST_Type::SIZE_TYPE size);

  bool gen_typedef(UTL_ScopedName* name, AST_Type* type, const char* repoid);

  bool gen_union(UTL_ScopedName* name,
                 const std::vector<AST_UnionBranch*>& branches,
                 AST_Type* type, const char* repoid);
};

#endif