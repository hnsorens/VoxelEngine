#include <tuple>
#include <vector>
#include <type_traits>
#include <string>

enum BindingType {
    BINDING_IMAGE,
    BINDING_BUFFER,
};

class Binding {
    virtual BindingType bindingType();
    virtual int binding();
};

template <BindingType ResourceType, int b>
struct BindingBase : Binding
{
    static constexpr BindingType type() { return ResourceType; }
    static constexpr int get_binding() { return b; }

    BindingType bindingType() override { return type(); }
    int binding() override { return get_binding(); }
    // using type = Info;
};

template <typename T>
inline constexpr bool is_binding_v =
    std::is_base_of_v<BindingBase<T::type(), T::get_binding()>, T>;

struct ImageBindingInfo
{

};

template <int binding>
struct ImageBinding : BindingBase<BINDING_IMAGE, binding>
{
    using infoType = ImageBindingInfo;
};

struct BufferBindingInfo
{

};

template <int binding>
struct BufferBinding : BindingBase<BINDING_BUFFER, binding>
{
    using infoType = BufferBindingInfo;
};

template <int binding>
struct BindingSlot {
    template <typename T>
    struct Bind {
        T info;
        static constexpr int get_binding() { return binding; }
    };
};

template <typename type, int binding>
struct Bind {
    static_assert(is_binding_v<type>);
    using infoType = typename type::infoType;
    static constexpr BindingType get_type() { return type::type(); }
    static constexpr int get_binding() { return binding; }
};

template <int binding, typename Tuple>
struct find_binding;

template <int binding, typename First, typename... Rest>
struct find_binding<binding, std::tuple<First, Rest...>> {
    using type = typename std::conditional_t<
        (First::get_binding() == binding),
        First,
        typename find_binding<binding, std::tuple<Rest...>>::type
    >;
};

template <int binding>
struct find_binding<binding, std::tuple<>> {
    using type = void;
};

// Check if a binding exists in a tuple
template <int binding, typename Tuple>
struct has_binding {
    static constexpr bool value = 
        !std::is_same_v<typename find_binding<binding, Tuple>::type, void>;
};

// Get the type of a binding in a tuple
template <int binding, typename Tuple>
using get_binding_type = typename find_binding<binding, Tuple>::type;

template <typename... Shaders>
struct has_duplicate_shader_types;

template <>
struct has_duplicate_shader_types<> {
    static constexpr bool value = false;
};

template <typename First, typename... Rest>
struct has_duplicate_shader_types<First, Rest...>
{
    static constexpr bool value = 
        ((First::get_type() == Rest::get_type()) || ...) ||
        has_duplicate_shader_types<Rest...>::value;
};

template <typename... Shaders>
struct all_shader_types_unique {
    static constexpr bool value = !has_duplicate_shader_types<Shaders...>::value;
};

// Combine bindings from multiple shaders
template <typename... Shaders>
struct CombinedBindings {
    using AllBindings = decltype(std::tuple_cat(typename Shaders::BindingsList{}...));
    
    template <typename Tuple, typename Result = std::tuple<>>
    struct filter_duplicates;
    
    template <typename... Result>
    struct filter_duplicates<std::tuple<>, std::tuple<Result...>> {
        using type = std::tuple<Result...>;
    };
    
    template <typename First, typename... Rest, typename... Result>
    struct filter_duplicates<std::tuple<First, Rest...>, std::tuple<Result...>> {
        static constexpr int current_binding = First::get_binding();
        using type = typename std::conditional_t<
            has_binding<current_binding, std::tuple<Result...>>::value,
            typename filter_duplicates<std::tuple<Rest...>, std::tuple<Result...>>::type,
            typename filter_duplicates<std::tuple<Rest...>, std::tuple<Result..., First>>::type
        >;
    };
    
    using type = typename filter_duplicates<AllBindings>::type;
};

// Validate that matching bindings have the same type
template <typename... Shaders>
struct validate_shader_bindings {
    template <typename Tuple>
    struct check_conflicts;
    
    template <typename... Bindings>
    struct check_conflicts<std::tuple<Bindings...>> {
        template <typename Binding>
        struct check_binding {
            static constexpr bool value = []{
                constexpr int binding = Binding::get_binding();
                using CurrentType = Binding;
                
                bool valid = true;
                using ShaderList = std::tuple<Shaders...>;
                
                // Check each shader for this binding
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    ([&] {
                        using Shader = std::tuple_element_t<Is, ShaderList>;
                        if constexpr (has_binding<binding, typename Shader::BindingsList>::value) {
                            using ShaderBindingType = get_binding_type<binding, typename Shader::BindingsList>;
                            valid = valid && std::is_same_v<CurrentType, ShaderBindingType>;
                        }
                    }(), ...);
                }(std::make_index_sequence<sizeof...(Shaders)>{});
                
                return valid;
            }();
        };
        
        static constexpr bool value = (check_binding<Bindings>::value && ...);
    };
    
    static constexpr bool value = 
        check_conflicts<typename CombinedBindings<Shaders...>::type>::value;
};

template <typename Tuple>
struct BindingResources;

template <typename... Bindings>
struct BindingResources<std::tuple<Bindings...>> {
    template <typename B>
    struct BindingToSlot {
        using type = typename BindingSlot<B::get_binding()>::template Bind<typename B::infoType>;
    };

    using type = typename std::tuple<typename BindingToSlot<Bindings>::type...>;
};

enum ShaderType
{
    SHADER_VERTEX,
    SHADER_FRAGMENT,
    SHADER_GEOMETRY,
};

template <ShaderType Type, typename... Bindings>
class Shader {
public:
    using BindingsList = std::tuple<Bindings...>;

    Shader(const std::string& name, const std::string& path) {}

    static constexpr ShaderType get_type() { return Type; }
};

class ShaderPipeline
{

};

template <typename... Shaders>
class ShaderPipelineBuilder {
    static_assert(all_shader_types_unique<Shaders...>::value,
                 "Shader types conflict - multiple shaders with the same shader type");
    static_assert(validate_shader_bindings<Shaders...>::value,
                 "Shader bindings conflict - same binding number with different resource type");
    
public:
    using UniqueBindings = typename CombinedBindings<Shaders...>::type;
    using UniqueBindingResources = typename BindingResources<UniqueBindings>::type;
    
    ShaderPipelineBuilder(Shaders&... shaders) : m_shaders(shaders...) {}

    ShaderPipeline* build(UniqueBindingResources bindings)
    {
        
    }
    
private:
    std::tuple<Shaders&...> m_shaders;
};

int main() {
    // These will compile (compatible bindings)
    Shader<
        SHADER_VERTEX,
        ImageBinding<1>,
        BufferBinding<2>
    > shader1("shader1", "path1");
    
    Shader<
        SHADER_FRAGMENT,
        ImageBinding<1>,  // Same type as shader1's binding 1
        BufferBinding<3>  // Different binding number
    > shader2("shader2", "path2");
    
    ShaderPipelineBuilder pipelineBuilder(shader1, shader2);

    pipelineBuilder.build({
        BindingSlot<1>::Bind{ImageBindingInfo{}}, 
        BindingSlot<2>::Bind{BufferBindingInfo{}}, 
        BindingSlot<3>::Bind{BufferBindingInfo{}}
    });
    
    return 0;
}