# File format

### How to compile

Compilation of the plugin and utility programs is done automatically after kmtricks compilation.

Currently, XZ Utils is pulled from a git repository with CMake. In kmtricks, SDSL is built as a static library (.a) and compilation fail because, as plugin is a shared library, SDSL must be compiled with with position independent code (fPIC).

### Partition

Compressed partitions are headerless and contain many compressed blocks. Starting positions of compressed blocks are encoded using an Elias-Fano representation (as they are a strictly increasing sequence of integers) which is in a side file. Each partition has a side file, side files also contain the minimum hash value of its corresponding partition and the number of encoded integers.

### Side file
<table>
    <tr>
        <td><b>Field</b></td>
        <td>Minimum hash</td>
        <td>Number of integers stored in Elias-Fano</td>
        <td>Serialized Elias-Fano object</td>
    </tr>
    <tr>
    <td><b>Type</b></td>
    <td>uint64_t</td>
    <td>uint64_t</td>
    <td>sdsl::sd_vector&lt;&gt;</td>
    </tr>
    <tr>
    <td><b>Bytes</b></td>
    <td>8</td>
    <td>8</td>
    <td>-</td>
    </tr>

</table>
