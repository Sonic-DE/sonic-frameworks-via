# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: SonicDE

include(ECMAddTests)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Test)

ecm_add_test(test_command.cpp
    TEST_NAME sonicvia_test_command
    LINK_LIBRARIES SonicVia Qt6::Test
)
ecm_add_test(test_protocol.cpp
    TEST_NAME sonicvia_test_protocol
    LINK_LIBRARIES SonicVia Qt6::Test
)
ecm_add_test(test_discovery.cpp
    TEST_NAME sonicvia_test_discovery
    LINK_LIBRARIES SonicVia Qt6::Test
)
ecm_add_test(test_color.cpp
    TEST_NAME sonicvia_test_color
    LINK_LIBRARIES SonicVia Qt6::Test
)
