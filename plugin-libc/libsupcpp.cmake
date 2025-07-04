set(libsupcpp ${CMAKE_CURRENT_LIST_DIR}/libstdc++-v3/libsupc++)

set(LIBSUPCPP_SOURCES
	# ${libsupcpp}/array_type_info.cc
	${libsupcpp}/atexit_arm.cc
	${libsupcpp}/atexit_thread.cc
	${libsupcpp}/atomicity.cc 
	${libsupcpp}/bad_alloc.cc
	${libsupcpp}/bad_array_length.cc
	${libsupcpp}/bad_array_new.cc
	${libsupcpp}/bad_cast.cc
	${libsupcpp}/bad_typeid.cc
	${libsupcpp}/class_type_info.cc
	${libsupcpp}/del_op.cc
	${libsupcpp}/del_ops.cc
	${libsupcpp}/del_opnt.cc
	${libsupcpp}/del_opv.cc
	${libsupcpp}/del_opvs.cc
	${libsupcpp}/del_opvnt.cc
	${libsupcpp}/dyncast.cc
	# ${libsupcpp}/eh_alloc.cc
	# ${libsupcpp}/eh_arm.cc
	# ${libsupcpp}/eh_aux_runtime.cc
	# ${libsupcpp}/eh_call.cc
	# ${libsupcpp}/eh_catch.cc
	${libsupcpp}/eh_exception.cc
	# ${libsupcpp}/eh_globals.cc
	# ${libsupcpp}/eh_personality.cc
	# ${libsupcpp}/eh_ptr.cc
	# ${libsupcpp}/eh_term_handler.cc
	# ${libsupcpp}/eh_terminate.cc
	# ${libsupcpp}/eh_tm.cc
	# ${libsupcpp}/eh_throw.cc
	# ${libsupcpp}/eh_type.cc
	# ${libsupcpp}/eh_unex_handler.cc
	${libsupcpp}/enum_type_info.cc
	${libsupcpp}/function_type_info.cc
	${libsupcpp}/fundamental_type_info.cc
	${libsupcpp}/guard.cc
	# ${libsupcpp}/guard_error.cc
	${libsupcpp}/hash_bytes.cc
	# ${libsupcpp}/nested_exception.cc
	${libsupcpp}/new_handler.cc
	${libsupcpp}/new_op.cc
	${libsupcpp}/new_opnt.cc
	${libsupcpp}/new_opv.cc
	${libsupcpp}/new_opvnt.cc
	${libsupcpp}/new_opa.cc
	${libsupcpp}/new_opant.cc
	${libsupcpp}/new_opva.cc
	${libsupcpp}/new_opvant.cc
	${libsupcpp}/del_opa.cc
	${libsupcpp}/del_opant.cc
	${libsupcpp}/del_opsa.cc
	${libsupcpp}/del_opva.cc
	${libsupcpp}/del_opvant.cc
	${libsupcpp}/del_opvsa.cc
	${libsupcpp}/pbase_type_info.cc
	${libsupcpp}/pmem_type_info.cc
	${libsupcpp}/pointer_type_info.cc
	${libsupcpp}/pure.cc
	${libsupcpp}/si_class_type_info.cc
	${libsupcpp}/tinfo.cc
	${libsupcpp}/tinfo2.cc
	${libsupcpp}/vec.cc
	${libsupcpp}/vmi_class_type_info.cc
	# ${libsupcpp}/vterminate.cc
)

target_sources(metamodule-plugin-libc PRIVATE
    ${LIBSUPCPP_SOURCES}
)
