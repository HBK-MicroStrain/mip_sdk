
#include <microstrain/array_view.hpp>

#include <microstrain_test/microstrain_test.hpp>


using namespace microstrain;


MICROSTRAIN_TEST_CASE("C++ ArrayView", "An ArrayView constructed from a pointer and length has correct pointer and size")
{
    char view_buffer[] = "This is a test";
    const char* const_ptr = view_buffer;

    const ArrayView<      char   > view_mv(view_buffer, sizeof(view_buffer));
    const ArrayView<      char, 4> view_mf(view_buffer);
    const ArrayView<const char   > view_cv(view_buffer, sizeof(view_buffer));
    const ArrayView<const char, 4> view_cf(view_buffer);
    const ArrayView<const char   > view_ccv(const_ptr, sizeof(view_buffer));
    const ArrayView<const char, 4> view_ccf(const_ptr);

    CHECK_EQ(view_mv.data(), view_buffer);
    CHECK_EQ(view_mf.data(), view_buffer);
    CHECK_EQ(view_cv.data(), view_buffer);
    CHECK_EQ(view_cf.data(), view_buffer);
    CHECK_EQ(view_ccv.data(), view_buffer);
    CHECK_EQ(view_ccf.data(), view_buffer);
    CHECK_EQ(view_mv.size(), sizeof(view_buffer));
    CHECK_EQ(view_mf.size(), 4);
    CHECK_EQ(view_cv.size(), sizeof(view_buffer));
    CHECK_EQ(view_cf.size(), 4);
    CHECK_EQ(view_ccv.size(), sizeof(view_buffer));
    CHECK_EQ(view_ccf.size(), 4);
}

MICROSTRAIN_TEST_CASE("C++ ArrayView", "An ArrayView constructed from another ArrayView has correct pointer and size")
{
    char view_buffer[] = "This is a test";
    const ArrayView<      char   > view_mv(view_buffer, sizeof(view_buffer));
    const ArrayView<      char, 4> view_mf(view_buffer);
    const ArrayView<const char   > view_cv(view_buffer, sizeof(view_buffer));
    const ArrayView<const char, 4> view_cf(view_buffer);

    ArrayView<char         > copy_mv(view_mv);
    ArrayView<char, 4      > copy_mf(view_mf);
    ArrayView<const char   > copy_cv(view_cv);
    ArrayView<const char, 4> copy_cf(view_cf);
    ArrayView<const char   > copy_cmv(view_mv);
    ArrayView<const char, 4> copy_cmf(view_mf);

    CHECK_EQ(copy_mv.data(), view_buffer);
    CHECK_EQ(copy_mf.data(), view_buffer);
    CHECK_EQ(copy_cv.data(), view_buffer);
    CHECK_EQ(copy_cf.data(), view_buffer);
    CHECK_EQ(copy_cmv.data(), view_buffer);
    CHECK_EQ(copy_cmf.data(), view_buffer);
    CHECK_EQ(copy_mv.size(), sizeof(view_buffer));
    CHECK_EQ(copy_mf.size(), 4);
    CHECK_EQ(copy_cv.size(), sizeof(view_buffer));
    CHECK_EQ(copy_cf.size(), 4);
    CHECK_EQ(copy_cmv.size(), sizeof(view_buffer));
    CHECK_EQ(copy_cmf.size(), 4);
}

MICROSTRAIN_TEST_CASE("C++ ArrayView", "An ArrayView constructed from a std::array has correct pointer and size")
{
    std::array<char, 4> view_buffer{};
    const std::array<char,4>& const_buffer = view_buffer;

    const ArrayView<      char   > view_mv(view_buffer);
    const ArrayView<      char, 4> view_mf(view_buffer);
    const ArrayView<const char   > view_cmv(view_buffer);
    const ArrayView<const char, 4> view_cmf(view_buffer);
    const ArrayView<const char   > view_ccv(const_buffer);
    const ArrayView<const char, 4> view_ccf(const_buffer);

    CHECK_EQ(view_mv.data(), view_buffer.data());
    CHECK_EQ(view_mf.data(), view_buffer.data());
    CHECK_EQ(view_cmv.data(), view_buffer.data());
    CHECK_EQ(view_cmf.data(), view_buffer.data());
    CHECK_EQ(view_ccv.data(), view_buffer.data());
    CHECK_EQ(view_ccf.data(), view_buffer.data());

    CHECK_EQ(view_mv.size(), view_buffer.size());
    CHECK_EQ(view_mf.size(), view_buffer.size());
    CHECK_EQ(view_cmv.size(), view_buffer.size());
    CHECK_EQ(view_cmf.size(), view_buffer.size());
    CHECK_EQ(view_ccv.size(), view_buffer.size());
    CHECK_EQ(view_ccf.size(), view_buffer.size());
}
