//===-- AppleGetItemInfoHandler.h ----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef lldb_AppleGetItemInfoHandler_h_
#define lldb_AppleGetItemInfoHandler_h_

// C Includes
// C++ Includes
#include <map>
#include <mutex>
#include <vector>

// Other libraries and framework includes
// Project includes
#include "lldb/lldb-public.h"
#include "lldb/Core/Error.h"
#include "lldb/Expression/UtilityFunction.h"
#include "lldb/Symbol/CompilerType.h"

// This class will insert a UtilityFunction into the inferior process for
// calling libBacktraceRecording's __introspection_dispatch_queue_item_get_info()
// function.  The function in the inferior will return a struct by value
// with these members:
//
//     struct get_item_info_return_values
//     {
//         introspection_dispatch_item_info_ref *item_buffer;
//         uint64_t item_buffer_size;
//     };
//
// The item_buffer pointer is an address in the inferior program's address
// space (item_buffer_size in size) which must be mach_vm_deallocate'd by
// lldb.  
//
// The AppleGetItemInfoHandler object should persist so that the UtilityFunction
// can be reused multiple times.

namespace lldb_private
{

class AppleGetItemInfoHandler {
public:

    AppleGetItemInfoHandler (lldb_private::Process *process);

    ~AppleGetItemInfoHandler();

    struct GetItemInfoReturnInfo
    {
        lldb::addr_t    item_buffer_ptr;  /* the address of the item buffer from libBacktraceRecording */
        lldb::addr_t    item_buffer_size; /* the size of the item buffer from libBacktraceRecording */

        GetItemInfoReturnInfo() :
            item_buffer_ptr(LLDB_INVALID_ADDRESS),
            item_buffer_size(0)
        {}
    };

    //----------------------------------------------------------
    /// Get the information about a work item by calling
    /// __introspection_dispatch_queue_item_get_info.  If there's a page of
    /// memory that needs to be freed, pass in the address and size and it will
    /// be freed before getting the list of queues.
    ///
    /// @param [in] thread
    ///     The thread to run this plan on.
    ///
    /// @param [in] item
    ///     The introspection_dispatch_item_info_ref value for the item of interest.
    ///
    /// @param [in] page_to_free
    ///     An address of an inferior process vm page that needs to be deallocated,
    ///     LLDB_INVALID_ADDRESS if this is not needed.
    ///
    /// @param [in] page_to_free_size
    ///     The size of the vm page that needs to be deallocated if an address was
    ///     passed in to page_to_free.
    ///
    /// @param [out] error
    ///     This object will be updated with the error status / error string from any failures encountered.
    ///
    /// @returns
    ///     The result of the inferior function call execution.  If there was a failure of any kind while getting
    ///     the information, the item_buffer_ptr value will be LLDB_INVALID_ADDRESS.
    //----------------------------------------------------------
    GetItemInfoReturnInfo
    GetItemInfo (Thread &thread, lldb::addr_t item, lldb::addr_t page_to_free, uint64_t page_to_free_size, lldb_private::Error &error);


    void
    Detach ();

private:

    lldb::addr_t
    SetupGetItemInfoFunction (Thread &thread, ValueList &get_item_info_arglist);

    static const char *g_get_item_info_function_name;
    static const char *g_get_item_info_function_code;

    lldb_private::Process *m_process;
    std::unique_ptr<UtilityFunction> m_get_item_info_impl_code;
    std::mutex m_get_item_info_function_mutex;

    lldb::addr_t m_get_item_info_return_buffer_addr;
    std::mutex m_get_item_info_retbuffer_mutex;
};

}  // using namespace lldb_private

#endif	// lldb_AppleGetItemInfoHandler_h_
