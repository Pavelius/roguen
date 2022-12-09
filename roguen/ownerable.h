#pragma once

class creature;

class ownerable {
	short unsigned	owner_id;
public:
	creature*	getowner() const;
	void		setowner(const creature* v);
};
