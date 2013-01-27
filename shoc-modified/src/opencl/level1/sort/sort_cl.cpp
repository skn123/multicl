const char *cl_source_sort =
"#define FPTYPE uint\n"
"#define FPVECTYPE uint4\n"
"\n"
"#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable \n"
"\n"
"\n"
"// Compute a per block histogram of the occurrences of each\n"
"// digit, using a 4-bit radix (i.e. 16 possible digits).\n"
"__kernel void\n"
"reduce(__global const FPTYPE * in, \n"
"       __global FPTYPE * isums, \n"
"       const int n,\n"
"       __local FPTYPE * lmem,\n"
"       const int shift) \n"
"{\n"
"    // First, calculate the bounds of the region of the array \n"
"    // that this block will sum.  We need these regions to match\n"
"    // perfectly with those in the bottom-level scan, so we index\n"
"    // as if vector types of length 4 were in use.  This prevents\n"
"    // errors due to slightly misaligned regions.\n"
"    int region_size = ((n / 4) / get_num_groups(0)) * 4;\n"
"    int block_start = get_group_id(0) * region_size;\n"
"\n"
"    // Give the last block any extra elements\n"
"    int block_stop  = (get_group_id(0) == get_num_groups(0) - 1) ? \n"
"        n : block_start + region_size;\n"
"\n"
"    // Calculate starting index for this thread/work item\n"
"    int tid = get_local_id(0);\n"
"    int i = block_start + tid;\n"
"    \n"
"    // The per thread histogram, initially 0's.\n"
"    int digit_counts[16] = { 0, 0, 0, 0, 0, 0, 0, 0,\n"
"                             0, 0, 0, 0, 0, 0, 0, 0 };\n"
"\n"
"    // Reduce multiple elements per thread\n"
"    while (i < block_stop)\n"
"    {\n"
"        // This statement \n"
"        // 1) Loads the value in from global memory\n"
"        // 2) Shifts to the right to have the 4 bits of interest\n"
"        //    in the least significant places\n"
"        // 3) Masks any more significant bits away. This leaves us\n"
"        // with the relevant digit (which is also the index into the\n"
"        // histogram). Next increment the histogram to count this occurrence.\n"
"        digit_counts[(in[i] >> shift) & 0xFU]++;\n"
"        i += get_local_size(0);\n"
"    }\n"
"    \n"
"    for (int d = 0; d < 16; d++)\n"
"    {\n"
"        // Load this thread's sum into local/shared memory\n"
"        lmem[tid] = digit_counts[d];\n"
"        barrier(CLK_LOCAL_MEM_FENCE);\n"
"\n"
"        // Reduce the contents of shared/local memory\n"
"        for (unsigned int s = get_local_size(0) / 2; s > 0; s >>= 1)\n"
"        {\n"
"            if (tid < s)\n"
"            {\n"
"                lmem[tid] += lmem[tid + s];\n"
"            }\n"
"            barrier(CLK_LOCAL_MEM_FENCE);\n"
"        }\n"
"\n"
"        // Write result for this block to global memory\n"
"        if (tid == 0)\n"
"        {\n"
"            isums[(d * get_num_groups(0)) + get_group_id(0)] = lmem[0];\n"
"        }\n"
"    }\n"
"}\n"
"\n"
"// This kernel scans the contents of local memory using a work\n"
"// inefficient, but highly parallel Kogge-Stone style scan.\n"
"// Set exclusive to 1 for an exclusive scan or 0 for an inclusive scan\n"
"inline FPTYPE scanLocalMem(FPTYPE val, __local FPTYPE* lmem, int exclusive)\n"
"{\n"
"    // Set first half of local memory to zero to make room for scanning\n"
"    int idx = get_local_id(0);\n"
"    lmem[idx] = 0;\n"
"    \n"
"    // Set second half to block sums from global memory, but don't go out\n"
"    // of bounds\n"
"    idx += get_local_size(0);\n"
"    lmem[idx] = val;\n"
"    barrier(CLK_LOCAL_MEM_FENCE);\n"
"    \n"
"    // Now, perform Kogge-Stone scan\n"
"    FPTYPE t;\n"
"    for (int i = 1; i < get_local_size(0); i *= 2)\n"
"    {\n"
"        t = lmem[idx -  i]; barrier(CLK_LOCAL_MEM_FENCE);\n"
"        lmem[idx] += t;     barrier(CLK_LOCAL_MEM_FENCE);\n"
"    }\n"
"    return lmem[idx-exclusive];\n"
"}\n"
"\n"
"// This single group kernel takes the per block histograms\n"
"// from the reduction and performs an exclusive scan on them.\n"
"__kernel void\n"
"top_scan(__global FPTYPE * isums, \n"
"         const int n,\n"
"         __local FPTYPE * lmem)\n"
"{\n"
"    __local int s_seed;\n"
"    s_seed = 0; barrier(CLK_LOCAL_MEM_FENCE);\n"
"    \n"
"    // Decide if this is the last thread that needs to \n"
"    // propagate the seed value\n"
"    int last_thread = (get_local_id(0) < n &&\n"
"                      (get_local_id(0)+1) == n) ? 1 : 0;\n"
"\n"
"    for (int d = 0; d < 16; d++)\n"
"    {\n"
"        FPTYPE val = 0;\n"
"        // Load each block's count for digit d\n"
"        if (get_local_id(0) < n)\n"
"        {\n"
"            val = isums[(n * d) + get_local_id(0)];\n"
"        }\n"
"        // Exclusive scan the counts in local memory\n"
"        FPTYPE res = scanLocalMem(val, lmem, 1);\n"
"        // Write scanned value out to global\n"
"        if (get_local_id(0) < n)\n"
"        {\n"
"            isums[(n * d) + get_local_id(0)] = res + s_seed;\n"
"        }\n"
"        \n"
"        if (last_thread) \n"
"        {\n"
"            s_seed += res + val;\n"
"        }\n"
"        barrier(CLK_LOCAL_MEM_FENCE);\n"
"    }\n"
"}\n"
"\n"
"\n"
"__kernel void \n"
"bottom_scan(__global const FPTYPE * in,\n"
"            __global const FPTYPE * isums,\n"
"            __global FPTYPE * out,\n"
"            const int n,\n"
"            __local FPTYPE * lmem,\n"
"            const int shift)\n"
"{\n"
"    // Use local memory to cache the scanned seeds\n"
"    __local FPTYPE l_scanned_seeds[16];\n"
"    \n"
"    // Keep a shared histogram of all instances seen by the current\n"
"    // block\n"
"    __local FPTYPE l_block_counts[16];\n"
"    \n"
"    // Keep a private histogram as well\n"
"    __private int histogram[16] = { 0, 0, 0, 0, 0, 0, 0, 0,\n"
"                          0, 0, 0, 0, 0, 0, 0, 0  };\n"
"\n"
"    // Prepare for reading 4-element vectors\n"
"    // Assume n is divisible by 4\n"
"    __global FPVECTYPE *in4  = (__global FPVECTYPE*) in;\n"
"    __global FPVECTYPE *out4 = (__global FPVECTYPE*) out;\n"
"    int n4 = n / 4; //vector type is 4 wide\n"
"    \n"
"    int region_size = n4 / get_num_groups(0);\n"
"    int block_start = get_group_id(0) * region_size;\n"
"    // Give the last block any extra elements\n"
"    int block_stop  = (get_group_id(0) == get_num_groups(0) - 1) ? \n"
"        n4 : block_start + region_size;\n"
"\n"
"    // Calculate starting index for this thread/work item\n"
"    int i = block_start + get_local_id(0);\n"
"    int window = block_start;\n"
"\n"
"    // Set the histogram in local memory to zero\n"
"    // and read in the scanned seeds from gmem\n"
"    if (get_local_id(0) < 16)\n"
"    {\n"
"        l_block_counts[get_local_id(0)] = 0;\n"
"        l_scanned_seeds[get_local_id(0)] = \n"
"            isums[(get_local_id(0)*get_num_groups(0))+get_group_id(0)];\n"
"    }\n"
"    barrier(CLK_LOCAL_MEM_FENCE);\n"
"\n"
"    // Scan multiple elements per thread\n"
"    while (window < block_stop)\n"
"    {\n"
"        // Reset histogram\n"
"        for (int q = 0; q < 16; q++) histogram[q] = 0;\n"
"        FPVECTYPE val_4;\n"
"        FPVECTYPE key_4;        \n"
"\n"
"        if (i < block_stop) // Make sure we don't read out of bounds\n"
"        {\n"
"            val_4 = in4[i];\n"
"            \n"
"            // Mask the keys to get the appropriate digit\n"
"            key_4.x = (val_4.x >> shift) & 0xFU;\n"
"            key_4.y = (val_4.y >> shift) & 0xFU;\n"
"            key_4.z = (val_4.z >> shift) & 0xFU;\n"
"            key_4.w = (val_4.w >> shift) & 0xFU;\n"
"            \n"
"            // Update the histogram\n"
"            histogram[key_4.x]++;\n"
"            histogram[key_4.y]++;\n"
"            histogram[key_4.z]++;\n"
"            histogram[key_4.w]++;\n"
"        } \n"
"                \n"
"        // Scan the digit counts in local memory\n"
"        for (int digit = 0; digit < 16; digit++)\n"
"        {\n"
"            histogram[digit] = scanLocalMem(histogram[digit], lmem, 1);\n"
"            barrier(CLK_LOCAL_MEM_FENCE);\n"
"        }\n"
"\n"
"        if (i < block_stop) // Make sure we don't write out of bounds\n"
"        {\n"
"            int address;\n"
"            address = histogram[key_4.x] + l_scanned_seeds[key_4.x] + l_block_counts[key_4.x];\n"
"            out[address] = val_4.x;\n"
"            histogram[key_4.x]++;\n"
"            \n"
"            address = histogram[key_4.y] + l_scanned_seeds[key_4.y] + l_block_counts[key_4.y];\n"
"            out[address] = val_4.y;\n"
"            histogram[key_4.y]++;\n"
"            \n"
"            address = histogram[key_4.z] + l_scanned_seeds[key_4.z] + l_block_counts[key_4.z];\n"
"            out[address] = val_4.z;\n"
"            histogram[key_4.z]++;\n"
"            \n"
"            address = histogram[key_4.w] + l_scanned_seeds[key_4.w] + l_block_counts[key_4.w];\n"
"            out[address] = val_4.w;\n"
"            histogram[key_4.w]++;\n"
"        }\n"
"                \n"
"        // Before proceeding, make sure everyone has finished their current\n"
"        // indexing computations.\n"
"        barrier(CLK_LOCAL_MEM_FENCE);\n"
"        // Now update the seed array.\n"
"        if (get_local_id(0) == get_local_size(0)-1)\n"
"        {\n"
"            for (int q = 0; q < 16; q++)\n"
"            {\n"
"                 l_block_counts[q] += histogram[q];\n"
"            }\n"
"        }\n"
"        barrier(CLK_LOCAL_MEM_FENCE);\n"
"        \n"
"        // Advance window\n"
"        window += get_local_size(0);\n"
"        i += get_local_size(0);\n"
"    }\n"
"}\n"
"\n"
;
