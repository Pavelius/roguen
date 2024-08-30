#pragma once

class creature;

struct ownerable {
	short unsigned	owner_id;
public:
	creature*	getowner() const;
	void		setowner(const creature* v);
};
