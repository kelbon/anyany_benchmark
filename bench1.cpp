
#include <benchmark/benchmark.h>

#include <random>
#include <memory>

struct IFooable {
  virtual int64_t foo(float) = 0;
  virtual std::unique_ptr<IFooable> clone() const = 0;
  virtual ~IFooable() = default;
};
// TODO add std::proxy in comparison
struct Deriv1 : IFooable {
  double x = 3.14;
  int64_t foo(float f) noexcept override {
    return static_cast<int>(x + f * 2);
  }
  std::unique_ptr<IFooable> clone() const override {
    return std::make_unique<Deriv1>(*this);
  }
  ~Deriv1() override = default;
};
struct Deriv2 : IFooable {
  int64_t foo(float f) noexcept override {
    return static_cast<int>(f * 12);
  }
  std::unique_ptr<IFooable> clone() const override {
    return std::make_unique<Deriv2>(*this);
  }
  ~Deriv2() override = default;
};
struct Deriv3 : IFooable {
  double x = 3.14;
  double z = 6.;
  int64_t foo(float f) noexcept override {
    return static_cast<int>(x + f * 2 + z);
  }
  std::unique_ptr<IFooable> clone() const override {
    return std::make_unique<Deriv3>(*this);
  }
  ~Deriv3() override = default;
};
#include <anyany.hpp>

trait(foo, int64_t(float), self.foo(args...));

using any_fooable = aa::any_with<foo, aa::copy, aa::move>;

struct concrete1 {
  double x = 3.14;
  int foo(float f) noexcept {
    return static_cast<int>(x + f * 2);
  }
};
struct concrete2 {
  int foo(float f) noexcept {
    return static_cast<int>(f * 12);
  }
};
struct concrete3 {
  double x = 3.14;
  double z = 6.;
  int foo(float f) noexcept {
    return static_cast<int>(x + f * 2 + z);
  }
};

static auto create_virtual(size_t count, std::default_random_engine& e) {
  std::vector<std::unique_ptr<IFooable>> vec;
  vec.reserve(count);
  auto generator = [&]() -> std::unique_ptr<IFooable> {
    switch (std::uniform_int_distribution<int>(0, 2)(e)) {
      case 0:
        return std::make_unique<Deriv1>();
      case 1:
        return std::make_unique<Deriv2>();
      case 2:
        return std::make_unique<Deriv3>();
    }
    _STL_UNREACHABLE;
  };
  std::ranges::generate_n(std::back_inserter(vec), count, generator);
  return vec;
}
static auto create_anyany(size_t count, std::default_random_engine& e) {
  std::vector<any_fooable> vec;
  vec.reserve(count);
  auto generator = [&]() -> any_fooable {
    switch (std::uniform_int_distribution<int>(0, 2)(e)) {
      case 0:
        return Deriv1{};
      case 1:
        return Deriv2{};
      case 2:
        return Deriv3{};
    }
    _STL_UNREACHABLE;
  };
  std::ranges::generate_n(std::back_inserter(vec), count, generator);
  return vec;
}

static auto bench_create_virtual(benchmark::State& state) {
  std::default_random_engine e(432423);
  for (auto _ : state) {
    auto x = create_virtual(std::uniform_int_distribution<size_t>(0, 100'000)(e), e);
    benchmark::DoNotOptimize(x);
  }
}
static auto bench_create_anyany(benchmark::State& state) {
  std::default_random_engine e(432423);
  for (auto _ : state) {
    auto x = create_anyany(std::uniform_int_distribution<size_t>(0, 100'000)(e), e);
    benchmark::DoNotOptimize(x);
  }
}
static void invoke_virtual(benchmark::State& state) {
  std::default_random_engine e(432423);
  auto vec = create_virtual(1'000'000, e);

  for (auto _ : state) {
    auto x = vec[std::uniform_int_distribution<size_t>(0, vec.size() - 1)(e)]->foo(13.3);
    benchmark::DoNotOptimize(x);
  }
}
static void invoke_anyany(benchmark::State& state) {
  std::default_random_engine e(432423);
  auto vec = create_anyany(1'000'000, e);

  for (auto _ : state) {
    auto x = vec[std::uniform_int_distribution<size_t>(0, vec.size() - 1)(e)].foo(13.3);
    benchmark::DoNotOptimize(x);
  }
}

static void copy_virtual(benchmark::State& state) {
  std::default_random_engine e(432423);
  auto vec = create_virtual(100'000, e);

  for (auto _ : state) {
    std::vector<std::unique_ptr<IFooable>> copy;
    copy.reserve(vec.size());
    for (auto& val : vec)
      copy.push_back(val->clone());
    benchmark::DoNotOptimize(copy);
  }
}
static void copy_anyany(benchmark::State& state) {
  std::default_random_engine e(432423);
  auto vec = create_anyany(100'000, e);

  for (auto _ : state) {
    auto copy = vec;
    benchmark::DoNotOptimize(copy);
  }
}

static void sort_virtual(benchmark::State& state) {
  std::default_random_engine e(432423);
  auto vec = create_virtual(100'000, e);

  for (auto _ : state) {
    std::ranges::sort(vec, std::ranges::less{}, [](auto& x) { return x->foo(10); });
    auto x = vec[vec.size() / 2]->foo(15);
    benchmark::DoNotOptimize(x);
  }
}
static void sort_anyany(benchmark::State& state) {
  std::default_random_engine e(432423);
  auto vec = create_anyany(100'000, e);

  for (auto _ : state) {
    std::ranges::sort(vec, std::ranges::less{}, [](auto& a) { return a.foo(10); });
    auto x = vec[vec.size() / 2].foo(15);
    benchmark::DoNotOptimize(x);
  }
}

BENCHMARK(bench_create_virtual);
BENCHMARK(bench_create_anyany);
BENCHMARK(invoke_virtual);
BENCHMARK(invoke_anyany);
BENCHMARK(copy_virtual);
BENCHMARK(copy_anyany);
BENCHMARK(sort_virtual);
BENCHMARK(sort_anyany);

BENCHMARK_MAIN();

/*
Выводы из бенчмарка:

1. Вызовы На малых количествах элементов в векторе перфоманс практически одинаковый(зависит от порядка тестов)
На больших значениях(когда элементы не влезают в кеш процессора) anyany начинает выигрывать за счёт лучшего расположения
элементов в памяти. vtable_ptr и value_ptr можно подгружать параллельно, тогда как с виртуальными функциями
процессор вынужден делать последовательно подгрузку сначала value_ptr, а затем vtable_ptr.
К тому же значение часто не нужно подгружать, потому что оно уже тут( а vtable с большой долей вероятности уже есть в кеше, так как мы работаем
с этими типами)
2. 

*/