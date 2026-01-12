/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */
#pragma once

#include <AzCore/base.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/functional.h>

namespace Sune
{
    /**
     * Macro to define a type-safe ID class for Sune.
     * This ensures that different ID types (SoundPlayerId, PlayerEffectId, etc.)
     * are distinct types in C++ and scripting systems (Lua/ScriptCanvas).
     *
     * Usage:
     *   AZ_SUNE_ID(SoundPlayerId, "{GUID}", "SoundPlayer")
     *
     * Parameters:
     *   - ClassName: The name of the ID class to create
     *   - TypeUuid: A unique GUID string in the format "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}"
     */
    #define AZ_SUNE_ID(ClassName, TypeUuid)                                  \
        class ClassName                                                                         \
        {                                                                                       \
        public:                                                                                 \
            static constexpr AZ::u64 Invalid##ClassName = 0xFFFFFFFFFFFFFFFFull;              \
                                                                                                \
            AZ_TYPE_INFO(ClassName, TypeUuid);                                                 \
                                                                                                \
            explicit AZ_FORCE_INLINE ClassName(AZ::u64 id = Invalid##ClassName)               \
                : m_id(id)                                                                      \
            {}                                                                                  \
                                                                                                \
            AZ_FORCE_INLINE explicit operator AZ::u64() const                                  \
            {                                                                                   \
                return m_id;                                                                    \
            }                                                                                   \
                                                                                                \
            AZ_FORCE_INLINE bool IsValid() const                                               \
            {                                                                                   \
                return m_id != Invalid##ClassName;                                             \
            }                                                                                   \
                                                                                                \
            AZ_FORCE_INLINE void SetInvalid()                                                  \
            {                                                                                   \
                m_id = Invalid##ClassName;                                                     \
            }                                                                                   \
                                                                                                \
            AZStd::string ToString() const                                                      \
            {                                                                                   \
                return AZStd::string::format( #ClassName "[%08llX]", m_id);                    \
            }                                                                                   \
                                                                                                \
            AZ_FORCE_INLINE bool operator==(const ClassName& rhs) const                        \
            {                                                                                   \
                return m_id == rhs.m_id;                                                        \
            }                                                                                   \
                                                                                                \
            AZ_FORCE_INLINE bool operator!=(const ClassName& rhs) const                        \
            {                                                                                   \
                return m_id != rhs.m_id;                                                        \
            }                                                                                   \
                                                                                                \
            AZ_FORCE_INLINE bool operator<(const ClassName& rhs) const                         \
            {                                                                                   \
                return m_id < rhs.m_id;                                                         \
            }                                                                                   \
                                                                                                \
            AZ_FORCE_INLINE bool operator>(const ClassName& rhs) const                         \
            {                                                                                   \
                return m_id > rhs.m_id;                                                         \
            }                                                                                   \
                                                                                                \
            AZ::u64 m_id;                                                                       \
        };

} // namespace Sune

/**
 * Macro to define hash specialization for Sune ID types.
 * Must be called in the global namespace after the ID class definition.
 *
 * Usage:
 *   AZ_SUNE_ID_HASH(Sune::SoundPlayerId)
 */
#define AZ_SUNE_ID_HASH(FullClassName)                                                    \
    namespace AZStd                                                                             \
    {                                                                                           \
        template<>                                                                              \
        struct hash<FullClassName>                                                              \
        {                                                                                       \
            typedef FullClassName argument_type;                                                \
            typedef AZStd::size_t result_type;                                                  \
                                                                                                \
            AZ_FORCE_INLINE size_t operator()(const FullClassName& id) const                   \
            {                                                                                   \
                AZStd::hash<AZ::u64> hasher;                                                    \
                return hasher(static_cast<AZ::u64>(id));                                        \
            }                                                                                   \
        };                                                                                      \
    }

/**
 * Helper macro to reflect a Sune ID type to BehaviorContext.
 * Call this within your Reflect() function's behaviorContext section.
 *
 * Usage:
 *   AZ_SUNE_ID_BEHAVIOR_REFLECT(SoundPlayerId, "Sune")
 */
#define AZ_SUNE_ID_BEHAVIOR_REFLECT(ClassName, ModuleName)                               \
    behaviorContext->Class<ClassName>()                                                         \
        ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value) \
        ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common) \
        ->Attribute(AZ::Script::Attributes::Module, ModuleName)                                \
        ->Method("IsValid", &ClassName::IsValid)                                               \
        ->Method("ToString", &ClassName::ToString)                                             \
            ->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::ToString) \
        ->Method("Equal", &ClassName::operator==)                                              \
            ->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::Equal)

/**
 * Helper macro to reflect a Sune ID type to SerializeContext.
 * Call this within your Reflect() function's serializeContext section.
 *
 * Usage:
 *   AZ_SUNE_ID_SERIALIZE_REFLECT(SoundPlayerId)
 */
#define AZ_SUNE_ID_SERIALIZE_REFLECT(ClassName)                                          \
    serializeContext->Class<ClassName>()                                                        \
        ->Version(1)                                                                            \
        ->Field("id", &ClassName::m_id)