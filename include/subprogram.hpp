#pragma once

#include <functional>
#include <memory>

namespace nfwk {

class loop;

class subprogram {
public:

	friend loop;

	subprogram(loop& loop);
	subprogram(const subprogram&) = delete;
	subprogram(subprogram&&) = delete;

	virtual ~subprogram();

	subprogram& operator=(const subprogram&) = delete;
	subprogram& operator=(subprogram&&) = delete;

	virtual void update() = 0;

	bool has_next_state() const;
	void change(const std::function<std::unique_ptr<subprogram>()>& make_subprogram);
	void stop();

	template<typename Subprogram>
	void change() {
		change([] {
			return std::make_unique<Subprogram>();
		});
	}

	loop& get_loop();
	float delta() const;

private:

	std::function<std::unique_ptr<subprogram>()> make_next_subprogram;
	loop* owning_loop{ nullptr };

};

}
