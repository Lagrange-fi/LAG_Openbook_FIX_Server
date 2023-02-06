set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library("sol_sdk" STATIC
        Hash.cpp
        PublicKey.cpp
        Keypair.cpp
        Instruction.cpp
        Transaction.cpp
)

target_include_directories("sol_sdk" PRIVATE
        "include"
        ".."
        )