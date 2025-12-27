#pragma once

struct creature;

struct ownerable {
	short unsigned	owner_id;
	creature*	getowner() const;
	void		setowner(const creature* v);
};
