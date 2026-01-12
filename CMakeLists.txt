# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
#
# For complete copyright and license terms please see the LICENSE at the root of this distribution.

o3de_gem_setup("Sune")

ly_add_external_target_path(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty)

add_subdirectory(Code)
