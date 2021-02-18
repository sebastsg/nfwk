export module nfwk.core:loop.component;

export namespace nfwk {

class program_state;

class loop_component {
public:

	enum class priority { force, highest, high, normal, low, lowest };

	loop_component(program_state& state) : state{ state } {}
	loop_component(const loop_component&) = delete;
	loop_component(loop_component&&) = delete;

	virtual ~loop_component() = default;

	loop_component& operator=(const loop_component&) = delete;
	loop_component& operator=(loop_component&&) = delete;

	virtual priority get_priority() const = 0;

	virtual void before() {}
	virtual void after() {}
	virtual void before_draw() {}
	virtual void after_draw() {}

protected:

	program_state& state;

};

}
