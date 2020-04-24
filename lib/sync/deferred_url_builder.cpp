#include "deferred_url_builder.hpp"

#include "../util/util.hpp"

constexpr std::string_view STATUS_ROUTE{ "/api/v1/statuses/" };
constexpr std::string_view MEDIA_ROUTE{ "/api/v1/media" };

const std::string& deferred_url_builder::make_if_empty(std::string& field, std::string_view route)
{
	if (field.empty())
		field = make_api_url(instance_url, route);
	return field;
}

const std::string& deferred_url_builder::status_url()
{
	return make_if_empty(cached_status_url, STATUS_ROUTE);
}

const std::string& deferred_url_builder::media_url()
{
	return make_if_empty(cached_media_url, MEDIA_ROUTE);
}
