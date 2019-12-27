#include "cxxopts.hpp"
#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <algorithm>
#include <map>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <cstdint>

#define RUNTIME_API_CALL(apiFuncCall)                                          \
do {                                                                           \
    cudaError_t _status = apiFuncCall;                                         \
    if (_status != cudaSuccess) {                                              \
        fprintf(stderr, "%s:%d: error: function %s failed with error %s.\n",   \
            __FILE__, __LINE__, #apiFuncCall, cudaGetErrorString(_status));    \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
} while (0)

#define ADD_KEY_AND_STR(x) {x, #x}

const std::map<cublasStatus_t, std::string> kErr2Str = {
    ADD_KEY_AND_STR(CUBLAS_STATUS_NOT_INITIALIZED),
    ADD_KEY_AND_STR(CUBLAS_STATUS_ALLOC_FAILED),
    ADD_KEY_AND_STR(CUBLAS_STATUS_INVALID_VALUE),
    ADD_KEY_AND_STR(CUBLAS_STATUS_ARCH_MISMATCH),
    ADD_KEY_AND_STR(CUBLAS_STATUS_MAPPING_ERROR),
    ADD_KEY_AND_STR(CUBLAS_STATUS_EXECUTION_FAILED),
    ADD_KEY_AND_STR(CUBLAS_STATUS_INTERNAL_ERROR),
    ADD_KEY_AND_STR(CUBLAS_STATUS_NOT_SUPPORTED),
    ADD_KEY_AND_STR(CUBLAS_STATUS_LICENSE_ERROR)
};

#define CUBLAS_API_CALL(apiFuncCall)                                            \
do {                                                                            \
    cublasStatus_t _status = apiFuncCall;                                       \
    if (_status != CUBLAS_STATUS_SUCCESS) {                                     \
        fprintf(stderr, "%s:%d: error: function %s failed with error %s.\n",    \
            __FILE__, __LINE__, #apiFuncCall, kErr2Str.at(_status).c_str());    \
        exit(EXIT_FAILURE);                                                     \
    }                                                                           \
} while (0)

const std::map<cudaDataType_t, std::string> kDtype2Str = {
    ADD_KEY_AND_STR(CUDA_R_8I),
    ADD_KEY_AND_STR(CUDA_R_16F),
    ADD_KEY_AND_STR(CUDA_R_32I),
    ADD_KEY_AND_STR(CUDA_R_32F),
    ADD_KEY_AND_STR(CUDA_R_64F),
    ADD_KEY_AND_STR(CUDA_C_8I),
    ADD_KEY_AND_STR(CUDA_C_32F),
    ADD_KEY_AND_STR(CUDA_C_64F)
};

const std::map<cudaDataType_t, int> kDtype2Size = {
    {CUDA_R_8I,   1},
    {CUDA_R_16F,  2},
    {CUDA_R_32I,  4},
    {CUDA_R_32F,  4},
    {CUDA_R_64F,  8},
    {CUDA_C_8I,   2},
    {CUDA_C_32F,  8},
    {CUDA_C_64F, 16}
};

const std::map<cublasOperation_t, std::string> kOperation2Str = {
    ADD_KEY_AND_STR(CUBLAS_OP_N),
    ADD_KEY_AND_STR(CUBLAS_OP_T)
};

const std::map<cublasGemmAlgo_t, std::string> kAlgo2Str = {
    ADD_KEY_AND_STR(CUBLAS_GEMM_DEFAULT),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO0),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO1),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO2),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO3),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO4),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO5),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO6),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO7),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO8),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO9),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO10),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO11),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO12),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO13),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO14),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO15),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO16),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO17),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO18),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO19),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO20),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO21),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO22),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO23),
    ADD_KEY_AND_STR(CUBLAS_GEMM_DEFAULT_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO0_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO1_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO2_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO3_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO4_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO5_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO6_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO7_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO8_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO9_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO10_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO11_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO12_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO13_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO14_TENSOR_OP),
    ADD_KEY_AND_STR(CUBLAS_GEMM_ALGO15_TENSOR_OP)
};

struct Dtypes_t {
    cudaDataType_t computeType;
    cudaDataType_t Atype;
    cudaDataType_t Btype;
    cudaDataType_t Ctype;
};

struct Param_t {
    cublasHandle_t handle;
    cublasOperation_t transa;
    cublasOperation_t transb;
    int m;
    int n;
    int k;
    void *alpha;
    void *A;
    int lda;
    void *B;
    int ldb;
    void *beta;
    void *C;
    int ldc;
    //cublasGemmAlgo_t algo;
    Dtypes_t dtype;
};

cxxopts::ParseResult Parse(int argc, const char* argv[]) {
  try {
    cxxopts::Options options(argv[0], "GEMM testing");

    options.positional_help("[optional args]").show_positional_help();

    options.add_options()
    ("m", "m dimension", cxxopts::value<int>()->default_value("32"))
    ("n", "n dimension", cxxopts::value<int>()->default_value("32"))
    ("k", "k dimension", cxxopts::value<int>()->default_value("32"))
    ("d", "device ID", cxxopts::value<int>()->default_value("0"))
    ("l", "loop", cxxopts::value<int>()->default_value("1"))
    ("ta", "set A to CUBLAS_OP_T, else CUBLAS_OP_N")
    ("tb", "set B to CUBLAS_OP_T, else CUBLAS_OP_N")
    ("type", "slect combination of types",
        cxxopts::value< std::vector<int> >()->default_value("5"))
    ("algo", "assgin algorithm ID (0~23)", cxxopts::value< std::vector<int> >())
    ("tensor_algo", "assgin TensorOp algorithm ID (0~15)", cxxopts::value< std::vector<int> >())
    ("all_algo", "run all algorithms")
    ("help", "print help");

    auto result = options.parse(argc, (char**&)argv);

    std::string type_info;
    type_info = "available combination of types:\n"
                "ID, ComputeType, Atype,      Btype,      Ctype\n"
                "0,  {CUDA_R_16F, CUDA_R_16F, CUDA_R_16F, CUDA_R_16F}\n"
                "1,  {CUDA_R_32I, CUDA_R_8I,  CUDA_R_8I,  CUDA_R_32I}\n"
                "2,  {CUDA_R_32F, CUDA_R_16F, CUDA_R_16F, CUDA_R_16F}\n"
                "3,  {CUDA_R_32F, CUDA_R_8I,  CUDA_R_8I,  CUDA_R_32F}\n"
                "4,  {CUDA_R_32F, CUDA_R_16F, CUDA_R_16F, CUDA_R_32F}\n"
                "5,  {CUDA_R_32F, CUDA_R_32F, CUDA_R_32F, CUDA_R_32F}\n"
                "6,  {CUDA_R_64F, CUDA_R_64F, CUDA_R_64F, CUDA_R_64F}\n"
                "7,  {CUDA_C_32F, CUDA_C_8I,  CUDA_C_8I,  CUDA_C_32F}\n"
                "8,  {CUDA_C_32F, CUDA_C_32F, CUDA_C_32F, CUDA_C_32F}\n";


    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        std::cout << type_info;
        exit(EXIT_SUCCESS);
    }

    return result;

  } catch (const cxxopts::OptionException& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

struct Result_t {
    cublasGemmAlgo_t algo;
    float time;
    float gflops;
    friend std::ostream& operator<<(std::ostream& os, const Result_t& x);
};

std::ostream& operator<<(std::ostream& os, const Result_t& x) {
    os << kAlgo2Str.at(x.algo) << ", " << x.time << ", " << x.gflops;
    return os;
}

bool SortResult (const Result_t& x, const Result_t& y) { return (x.time < y.time); }

void ProfileGemm(const Param_t& param, const std::vector<cublasGemmAlgo_t>& algos,
    const std::string& config_info, int loop) {

    cudaEvent_t start;
    cudaEvent_t end;
    cublasStatus_t ret;

    RUNTIME_API_CALL(cudaEventCreate(&start));
    RUNTIME_API_CALL(cudaEventCreate(&end));

    std::vector<Result_t> results;
    for (auto algo : algos) {

        //param.algo = algo;
        float time = 0.f;
        bool fault = false;

        RUNTIME_API_CALL(cudaEventRecord(start));
        for (int i = 0; i < loop; ++i) {
            ret = cublasGemmEx(param.handle,
                            param.transa, param.transb,
                            param.m, param.n, param.k,
                            param.alpha, param.A, param.dtype.Atype, param.lda,
                            param.B, param.dtype.Btype, param.ldb, param.beta,
                            param.C, param.dtype.Ctype, param.ldc,
                            param.dtype.computeType, algo);
            if (ret != CUBLAS_STATUS_SUCCESS && 
                ret != CUBLAS_STATUS_NOT_SUPPORTED &&
                ret != CUBLAS_STATUS_INVALID_VALUE) {
                fault = true;
                CUBLAS_API_CALL(ret);
                break;
            }
        }
        RUNTIME_API_CALL(cudaEventRecord(end));
        RUNTIME_API_CALL(cudaEventSynchronize(end));
        RUNTIME_API_CALL(cudaEventElapsedTime(&time, start, end));

        float gflops = 0;
        if (!fault) { 
            time /= loop;
            float workload = (2.f * param.m * param.n * param.k) * 1e-9;
            gflops = workload / (time * 1e-3);
        }
        else {
            time = NAN;
            gflops = NAN;
        }

        results.push_back(Result_t{algo, time, gflops});
    }

    RUNTIME_API_CALL(cudaEventDestroy(start));
    RUNTIME_API_CALL(cudaEventDestroy(end));
    std::cout << config_info << results[0] << std::endl;
    std::sort(results.begin(), results.end(), SortResult);
    std::cout << config_info << results[0] << std::endl;
}

std::string Mask2Str(const std::vector<bool>& mask) {
    std::string info;
    auto count = std::count_if(mask.begin(), mask.end(),
        [](bool x) { return x; });
    if (count == mask.size()) {
        info = "all meet, ";
    }
    else {
        info = "(";
        for (auto bit : mask) {
            info += std::to_string(static_cast<int>(bit)) + ".";
        }
        info += "), ";
    }
    return info;
}

std::string Dp4aRestrictions(const Param_t& param) {
    std::vector<bool> mask(2);
    mask[0] = param.lda % 4 == 0;
    mask[1] = param.ldb % 4 == 0;
    return Mask2Str(mask);
}

std::string TensorCoreRestrictions(const Param_t& param) {
    // refer to https://docs.nvidia.com/cuda/cublas/#tensorop-restrictions
    std::vector<bool> mask(8);
    mask[0] = param.m % 4 == 0;
    mask[1] = param.k % 8 == 0;
    mask[2] = reinterpret_cast<intptr_t>(param.A) % 16 == 0;
    mask[3] = reinterpret_cast<intptr_t>(param.B) % 16 == 0;
    mask[4] = reinterpret_cast<intptr_t>(param.C) % 16 == 0;
    mask[5] = param.lda % (16 / kDtype2Size.at(param.dtype.Atype)) == 0;
    mask[6] = param.ldb % (16 / kDtype2Size.at(param.dtype.Btype)) == 0;
    mask[7] = param.ldc % (16 / kDtype2Size.at(param.dtype.Ctype)) == 0;
    return Mask2Str(mask);
}

std::vector<cublasGemmAlgo_t> Int2Algo(const std::vector<int>& select_id, cublasGemmAlgo_t base) {
    std::vector<cublasGemmAlgo_t> algos;
    for (auto id : select_id) {
        algos.push_back(static_cast<cublasGemmAlgo_t>(id + static_cast<int>(base)));
    }
    return algos;
}

std::vector<cublasGemmAlgo_t> SetupAlgo(const cxxopts::ParseResult& parse, const char option[],
    const std::vector<cublasGemmAlgo_t>& all_options) {

    std::vector<cublasGemmAlgo_t> select_algo;
        if (parse.count("all_algo")) {
            select_algo = all_options;
        }
        else if (parse.count(option)) {
            auto select_id = parse[option].as< std::vector<int> >();
            select_algo = Int2Algo(select_id, CUBLAS_GEMM_ALGO0);
        }
        else {
            select_algo.push_back(CUBLAS_GEMM_DEFAULT);
        }
        return select_algo;
}

int main (int argc, const char* argv[]) {

    auto result = Parse(argc, argv);

    const std::vector<Dtypes_t> gemm_types = {
        Dtypes_t{CUDA_R_16F, CUDA_R_16F, CUDA_R_16F, CUDA_R_16F},
        Dtypes_t{CUDA_R_32I, CUDA_R_8I,  CUDA_R_8I,  CUDA_R_32I},
        Dtypes_t{CUDA_R_32F, CUDA_R_16F, CUDA_R_16F, CUDA_R_16F},
        Dtypes_t{CUDA_R_32F, CUDA_R_8I,  CUDA_R_8I,  CUDA_R_32F},
        Dtypes_t{CUDA_R_32F, CUDA_R_16F, CUDA_R_16F, CUDA_R_32F},
        Dtypes_t{CUDA_R_32F, CUDA_R_32F, CUDA_R_32F, CUDA_R_32F},
        Dtypes_t{CUDA_R_64F, CUDA_R_64F, CUDA_R_64F, CUDA_R_64F},
        Dtypes_t{CUDA_C_32F, CUDA_C_8I,  CUDA_C_8I,  CUDA_C_32F},
        Dtypes_t{CUDA_C_32F, CUDA_C_32F, CUDA_C_32F, CUDA_C_32F},
    };

    const std::vector<cublasGemmAlgo_t> cuda_algos = {
        CUBLAS_GEMM_DEFAULT,
        CUBLAS_GEMM_ALGO0,
        CUBLAS_GEMM_ALGO1,
        CUBLAS_GEMM_ALGO2,
        CUBLAS_GEMM_ALGO3,
        CUBLAS_GEMM_ALGO4,
        CUBLAS_GEMM_ALGO5,
        CUBLAS_GEMM_ALGO6,
        CUBLAS_GEMM_ALGO7,
        CUBLAS_GEMM_ALGO8,
        CUBLAS_GEMM_ALGO9,
        CUBLAS_GEMM_ALGO10,
        CUBLAS_GEMM_ALGO11,
        CUBLAS_GEMM_ALGO12,
        CUBLAS_GEMM_ALGO13,
        CUBLAS_GEMM_ALGO14,
        CUBLAS_GEMM_ALGO15,
        CUBLAS_GEMM_ALGO16,
        CUBLAS_GEMM_ALGO17,
        CUBLAS_GEMM_ALGO18,
        CUBLAS_GEMM_ALGO19,
        CUBLAS_GEMM_ALGO20,
        CUBLAS_GEMM_ALGO21,
        CUBLAS_GEMM_ALGO22,
        CUBLAS_GEMM_ALGO23
    };

    const std::vector<cublasGemmAlgo_t> tensor_algos = {
        CUBLAS_GEMM_DEFAULT_TENSOR_OP,
        CUBLAS_GEMM_ALGO0_TENSOR_OP,
        CUBLAS_GEMM_ALGO1_TENSOR_OP,
        CUBLAS_GEMM_ALGO2_TENSOR_OP,
        CUBLAS_GEMM_ALGO3_TENSOR_OP,
        CUBLAS_GEMM_ALGO4_TENSOR_OP,
        CUBLAS_GEMM_ALGO5_TENSOR_OP,
        CUBLAS_GEMM_ALGO6_TENSOR_OP,
        CUBLAS_GEMM_ALGO7_TENSOR_OP,
        CUBLAS_GEMM_ALGO8_TENSOR_OP,
        CUBLAS_GEMM_ALGO9_TENSOR_OP,
        CUBLAS_GEMM_ALGO10_TENSOR_OP,
        CUBLAS_GEMM_ALGO11_TENSOR_OP,
        CUBLAS_GEMM_ALGO12_TENSOR_OP,
        CUBLAS_GEMM_ALGO13_TENSOR_OP,
        CUBLAS_GEMM_ALGO14_TENSOR_OP,
        CUBLAS_GEMM_ALGO15_TENSOR_OP,
    };


    auto device_id = result["d"].as<int>();
    RUNTIME_API_CALL(cudaSetDevice(device_id));
    cudaDeviceProp prop;
    RUNTIME_API_CALL(cudaGetDeviceProperties(&prop, device_id));

    Param_t param;
    CUBLAS_API_CALL(cublasCreate(&param.handle));
    
    param.m = result["m"].as<int>();
    param.n = result["n"].as<int>();
    param.k = result["k"].as<int>();
    param.transa = result.count("ta") ? CUBLAS_OP_T : CUBLAS_OP_N;
    param.lda = (param.transa == CUBLAS_OP_N) ? param.m : param.k; 
    param.transb = result.count("tb") ? CUBLAS_OP_T : CUBLAS_OP_N;
    param.ldb = (param.transb == CUBLAS_OP_N) ? param.k : param.n;
    param.ldc = param.m;

    std::cout << "device, op(A), op(B), m, n, k, Atype, Btype, Ctype, "
        "ComputeType, Dp4aRestrictions(lda.ldb), TensorCoreRestrictions(m.k.A.B.C.lda.ldb.ldc), "
        "algo, time(ms), GFLOPS" << std::endl;

    std::string dims_info;
    dims_info = std::string(prop.name) + ", "
            + kOperation2Str.at(param.transa) + ", "
            + kOperation2Str.at(param.transb) + ", "
            + std::to_string(param.m) + ", "
            + std::to_string(param.n) + ", "
            + std::to_string(param.k) + ", ";

    auto selected_dtypes = result["type"].as< std::vector<int> >();

    for (auto dtype_id : selected_dtypes) {

        auto dtypes = gemm_types[dtype_id];
        param.dtype = dtypes;

        std::string all_info;
        all_info = dims_info
           + kDtype2Str.at(param.dtype.Atype) + ", "
           + kDtype2Str.at(param.dtype.Btype) + ", "
           + kDtype2Str.at(param.dtype.Ctype) + ", "
           + kDtype2Str.at(param.dtype.computeType) + ", ";
        if (param.dtype.computeType == CUDA_R_32I) {
            all_info += Dp4aRestrictions(param);
        }
        else {
            all_info += "NA, ";
        }

        auto src_dtype_size = kDtype2Size.at(dtypes.Atype);
        auto dst_dtype_size = kDtype2Size.at(dtypes.Ctype);

        void* dev_A;
        RUNTIME_API_CALL(cudaMalloc(&dev_A, param.m * param.k * src_dtype_size));
        void* dev_B;
        RUNTIME_API_CALL(cudaMalloc(&dev_B, param.k * param.n * src_dtype_size));
        void* dev_C;
        RUNTIME_API_CALL(cudaMalloc(&dev_C, param.m * param.n * dst_dtype_size));

        param.A = dev_A;
        param.B = dev_B;
        param.C = dev_C;

        auto compute_dtype_size = kDtype2Size.at(dtypes.computeType);
 
        char* host_alpha;
        host_alpha = reinterpret_cast<char*>(malloc(compute_dtype_size));
        memset(host_alpha, 0, compute_dtype_size);
        host_alpha[0] = 1; 

        char* host_beta;
        host_beta = reinterpret_cast<char*>(malloc(compute_dtype_size));
        memset(host_beta, 0, compute_dtype_size);

        param.alpha = (void*)host_alpha;
        param.beta  = (void*)host_beta;

        auto loop = result["l"].as<int>();

        auto select_algo = SetupAlgo(result, "algo", cuda_algos);
        ProfileGemm(param, select_algo, all_info + "NA, ", loop);

        if (prop.major > 6) {
            auto info = TensorCoreRestrictions(param);
            select_algo = SetupAlgo(result, "tensor_algo", tensor_algos);
            ProfileGemm(param, select_algo, all_info + info, loop);
        }

        RUNTIME_API_CALL(cudaFree(dev_A));
        RUNTIME_API_CALL(cudaFree(dev_B));
        RUNTIME_API_CALL(cudaFree(dev_C));
        free(host_alpha);
        free(host_beta);
    }

    CUBLAS_API_CALL(cublasDestroy(param.handle));
    return 0;
}